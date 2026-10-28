#pragma once

// Minimal QR Code Model 2 encoder (ISO/IEC 18004), byte mode, versions 1-40.
//
// Standard C++ only, with no engine types, so the exact same file is cross-checked outside
// the engine against an independent encoder (see README, "Tests"). The algorithm follows
// Project Nayuki's reference encoder (MIT), restricted to the one segment mode a URL needs.
// UInvoQrCodeLibrary (InvoQrCode.h) turns the module grid into a UTexture2D.

#include <cstdint>
#include <cstdlib>
#include <vector>

namespace InvoQr
{
    enum class EEcc : int { Low = 0, Medium = 1, Quartile = 2, High = 3 };

    class FQrCode
    {
    public:
        static constexpr int MinVersion = 1;
        static constexpr int MaxVersion = 40;

        /** Encodes Data in byte mode in the smallest version that fits. ForcedMask -1 picks the
         *  lowest-penalty mask (what scanners expect); 0-7 forces one (tests only).
         *  Returns false, leaving OutCode untouched, when the data is too long or Mask is invalid. */
        static bool EncodeBytes(const std::vector<uint8_t>& Data, EEcc Ecc, int ForcedMask, FQrCode& OutCode)
        {
            if (ForcedMask < -1 || ForcedMask > 7)
            {
                return false;
            }

            int Version = MinVersion;
            for (;; ++Version)
            {
                if (Version > MaxVersion)
                {
                    return false;
                }
                const int CcBits = Version <= 9 ? 8 : 16;
                if (Data.size() < (static_cast<size_t>(1) << CcBits))
                {
                    const int UsedBits = 4 + CcBits + static_cast<int>(Data.size()) * 8;
                    if (UsedBits <= NumDataCodewords(Version, Ecc) * 8)
                    {
                        break;
                    }
                }
            }

            std::vector<bool> Bits;
            AppendBits(Bits, 0x4, 4); // byte mode
            AppendBits(Bits, static_cast<int>(Data.size()), Version <= 9 ? 8 : 16);
            for (uint8_t B : Data)
            {
                AppendBits(Bits, B, 8);
            }

            const int CapacityBits = NumDataCodewords(Version, Ecc) * 8;
            const int Terminator = CapacityBits - static_cast<int>(Bits.size());
            AppendBits(Bits, 0, Terminator < 4 ? Terminator : 4);
            AppendBits(Bits, 0, (8 - static_cast<int>(Bits.size()) % 8) % 8);
            for (int Pad = 0xEC; static_cast<int>(Bits.size()) < CapacityBits; Pad ^= 0xEC ^ 0x11)
            {
                AppendBits(Bits, Pad, 8);
            }

            std::vector<uint8_t> Codewords(Bits.size() / 8, 0);
            for (size_t I = 0; I < Bits.size(); ++I)
            {
                if (Bits[I])
                {
                    Codewords[I >> 3] |= static_cast<uint8_t>(1 << (7 - (I & 7)));
                }
            }

            OutCode.Build(Version, Ecc, Codewords, ForcedMask);
            return true;
        }

        int GetVersion() const { return Version; }
        int GetSize() const { return Size; }
        int GetMask() const { return Mask; }

        /** True when the module at column X, row Y is dark. Outside the symbol reads as light. */
        bool IsDark(int X, int Y) const
        {
            return X >= 0 && X < Size && Y >= 0 && Y < Size && Modules[Y * Size + X];
        }

    private:
        int Version = 0;
        int Size = 0;
        int Mask = 0;
        EEcc Ecc = EEcc::Medium;
        std::vector<bool> Modules;
        std::vector<bool> IsFunction;

        void Build(int InVersion, EEcc InEcc, const std::vector<uint8_t>& DataCodewords, int ForcedMask)
        {
            Version = InVersion;
            Ecc = InEcc;
            Size = Version * 4 + 17;
            Modules.assign(Size * Size, false);
            IsFunction.assign(Size * Size, false);

            DrawFunctionPatterns();
            DrawCodewords(AddEccAndInterleave(DataCodewords));

            int Chosen = ForcedMask;
            if (Chosen == -1)
            {
                int MinPenalty = 0x7FFFFFFF;
                for (int I = 0; I < 8; ++I)
                {
                    ApplyMask(I);
                    DrawFormatBits(I);
                    const int Penalty = PenaltyScore();
                    if (Penalty < MinPenalty)
                    {
                        Chosen = I;
                        MinPenalty = Penalty;
                    }
                    ApplyMask(I); // XOR undoes it
                }
            }
            Mask = Chosen;
            ApplyMask(Mask);
            DrawFormatBits(Mask);
        }

        // ---------------- function patterns ----------------

        void DrawFunctionPatterns()
        {
            for (int I = 0; I < Size; ++I)
            {
                SetFunction(6, I, I % 2 == 0);
                SetFunction(I, 6, I % 2 == 0);
            }

            DrawFinder(3, 3);
            DrawFinder(Size - 4, 3);
            DrawFinder(3, Size - 4);

            const std::vector<int> Align = AlignmentPositions();
            const int N = static_cast<int>(Align.size());
            for (int I = 0; I < N; ++I)
            {
                for (int J = 0; J < N; ++J)
                {
                    if ((I == 0 && J == 0) || (I == 0 && J == N - 1) || (I == N - 1 && J == 0))
                    {
                        continue; // overlaps a finder
                    }
                    DrawAlignment(Align[I], Align[J]);
                }
            }

            DrawFormatBits(0); // placeholder so the area is marked as function modules
            DrawVersion();
        }

        void DrawFormatBits(int InMask)
        {
            const int Data = FormatBits(Ecc) << 3 | InMask;
            int Rem = Data;
            for (int I = 0; I < 10; ++I)
            {
                Rem = (Rem << 1) ^ ((Rem >> 9) * 0x537);
            }
            const int Bits = (Data << 10 | Rem) ^ 0x5412;

            for (int I = 0; I <= 5; ++I)
            {
                SetFunction(8, I, Bit(Bits, I));
            }
            SetFunction(8, 7, Bit(Bits, 6));
            SetFunction(8, 8, Bit(Bits, 7));
            SetFunction(7, 8, Bit(Bits, 8));
            for (int I = 9; I < 15; ++I)
            {
                SetFunction(14 - I, 8, Bit(Bits, I));
            }

            for (int I = 0; I < 8; ++I)
            {
                SetFunction(Size - 1 - I, 8, Bit(Bits, I));
            }
            for (int I = 8; I < 15; ++I)
            {
                SetFunction(8, Size - 15 + I, Bit(Bits, I));
            }
            SetFunction(8, Size - 8, true); // the dark module
        }

        void DrawVersion()
        {
            if (Version < 7)
            {
                return;
            }
            int Rem = Version;
            for (int I = 0; I < 12; ++I)
            {
                Rem = (Rem << 1) ^ ((Rem >> 11) * 0x1F25);
            }
            const int Bits = Version << 12 | Rem;
            for (int I = 0; I < 18; ++I)
            {
                const bool B = Bit(Bits, I);
                const int A = Size - 11 + I % 3;
                const int C = I / 3;
                SetFunction(A, C, B);
                SetFunction(C, A, B);
            }
        }

        void DrawFinder(int X, int Y)
        {
            for (int Dy = -4; Dy <= 4; ++Dy)
            {
                for (int Dx = -4; Dx <= 4; ++Dx)
                {
                    const int Dist = std::abs(Dx) > std::abs(Dy) ? std::abs(Dx) : std::abs(Dy);
                    const int Xx = X + Dx;
                    const int Yy = Y + Dy;
                    if (Xx >= 0 && Xx < Size && Yy >= 0 && Yy < Size)
                    {
                        SetFunction(Xx, Yy, Dist != 2 && Dist != 4);
                    }
                }
            }
        }

        void DrawAlignment(int X, int Y)
        {
            for (int Dy = -2; Dy <= 2; ++Dy)
            {
                for (int Dx = -2; Dx <= 2; ++Dx)
                {
                    const int Dist = std::abs(Dx) > std::abs(Dy) ? std::abs(Dx) : std::abs(Dy);
                    SetFunction(X + Dx, Y + Dy, Dist != 1);
                }
            }
        }

        void SetFunction(int X, int Y, bool bDark)
        {
            Modules[Y * Size + X] = bDark;
            IsFunction[Y * Size + X] = true;
        }

        std::vector<int> AlignmentPositions() const
        {
            if (Version == 1)
            {
                return std::vector<int>();
            }
            const int NumAlign = Version / 7 + 2;
            const int Step = (Version * 8 + NumAlign * 3 + 5) / (NumAlign * 4 - 4) * 2;
            std::vector<int> Result(NumAlign, 0);
            Result[0] = 6;
            for (int I = NumAlign - 1, Pos = Size - 7; I >= 1; --I, Pos -= Step)
            {
                Result[I] = Pos;
            }
            return Result;
        }

        // ---------------- codewords ----------------

        std::vector<uint8_t> AddEccAndInterleave(const std::vector<uint8_t>& Data) const
        {
            const int NumBlocks = NumErrorCorrectionBlocks(Ecc, Version);
            const int BlockEccLen = EccCodewordsPerBlock(Ecc, Version);
            const int RawCodewords = NumRawDataModules(Version) / 8;
            const int NumShortBlocks = NumBlocks - RawCodewords % NumBlocks;
            const int ShortBlockLen = RawCodewords / NumBlocks;

            const std::vector<uint8_t> Divisor = ReedSolomonDivisor(BlockEccLen);
            std::vector<std::vector<uint8_t>> Blocks;
            for (int I = 0, K = 0; I < NumBlocks; ++I)
            {
                const int DatLen = ShortBlockLen - BlockEccLen + (I < NumShortBlocks ? 0 : 1);
                const std::vector<uint8_t> Dat(Data.begin() + K, Data.begin() + K + DatLen);
                K += DatLen;
                const std::vector<uint8_t> EccBytes = ReedSolomonRemainder(Dat, Divisor);

                // Every block is padded to ShortBlockLen + 1 so the interleave is one loop;
                // the padding slot of a short block is skipped below.
                std::vector<uint8_t> Block(ShortBlockLen + 1, 0);
                for (int J = 0; J < DatLen; ++J)
                {
                    Block[J] = Dat[J];
                }
                for (int J = 0; J < BlockEccLen; ++J)
                {
                    Block[Block.size() - BlockEccLen + J] = EccBytes[J];
                }
                Blocks.push_back(Block);
            }

            std::vector<uint8_t> Result;
            Result.reserve(RawCodewords);
            for (int I = 0; I < ShortBlockLen + 1; ++I)
            {
                for (int J = 0; J < NumBlocks; ++J)
                {
                    if (I != ShortBlockLen - BlockEccLen || J >= NumShortBlocks)
                    {
                        Result.push_back(Blocks[J][I]);
                    }
                }
            }
            return Result;
        }

        void DrawCodewords(const std::vector<uint8_t>& Data)
        {
            size_t I = 0;
            for (int Right = Size - 1; Right >= 1; Right -= 2)
            {
                if (Right == 6)
                {
                    Right = 5; // skip the vertical timing pattern
                }
                for (int Vert = 0; Vert < Size; ++Vert)
                {
                    for (int J = 0; J < 2; ++J)
                    {
                        const int X = Right - J;
                        const bool bUpward = ((Right + 1) & 2) == 0;
                        const int Y = bUpward ? Size - 1 - Vert : Vert;
                        if (!IsFunction[Y * Size + X] && I < Data.size() * 8)
                        {
                            Modules[Y * Size + X] = Bit(Data[I >> 3], 7 - static_cast<int>(I & 7));
                            ++I;
                        }
                        // Remainder bits stay light, as initialised.
                    }
                }
            }
        }

        void ApplyMask(int InMask)
        {
            for (int Y = 0; Y < Size; ++Y)
            {
                for (int X = 0; X < Size; ++X)
                {
                    bool bInvert = false;
                    switch (InMask)
                    {
                    case 0: bInvert = (X + Y) % 2 == 0; break;
                    case 1: bInvert = Y % 2 == 0; break;
                    case 2: bInvert = X % 3 == 0; break;
                    case 3: bInvert = (X + Y) % 3 == 0; break;
                    case 4: bInvert = (X / 3 + Y / 2) % 2 == 0; break;
                    case 5: bInvert = X * Y % 2 + X * Y % 3 == 0; break;
                    case 6: bInvert = (X * Y % 2 + X * Y % 3) % 2 == 0; break;
                    default: bInvert = ((X + Y) % 2 + X * Y % 3) % 2 == 0; break;
                    }
                    const int Idx = Y * Size + X;
                    if (bInvert && !IsFunction[Idx])
                    {
                        Modules[Idx] = !Modules[Idx];
                    }
                }
            }
        }

        // ---------------- mask penalty (ISO/IEC 18004 section 7.8.3) ----------------

        int PenaltyScore() const
        {
            constexpr int N1 = 3, N2 = 3, N3 = 40, N4 = 10;
            int Result = 0;

            for (int Pass = 0; Pass < 2; ++Pass) // 0 = rows, 1 = columns
            {
                for (int A = 0; A < Size; ++A)
                {
                    bool bRunColor = false;
                    int RunLen = 0;
                    int History[7] = { 0, 0, 0, 0, 0, 0, 0 };
                    for (int B = 0; B < Size; ++B)
                    {
                        const bool bCell = Pass == 0 ? Modules[A * Size + B] : Modules[B * Size + A];
                        if (bCell == bRunColor)
                        {
                            ++RunLen;
                            if (RunLen == 5) Result += N1;
                            else if (RunLen > 5) ++Result;
                        }
                        else
                        {
                            AddHistory(RunLen, History);
                            if (!bRunColor)
                            {
                                Result += CountFinderLike(History) * N3;
                            }
                            bRunColor = bCell;
                            RunLen = 1;
                        }
                    }
                    Result += TerminateAndCount(bRunColor, RunLen, History) * N3;
                }
            }

            for (int Y = 0; Y < Size - 1; ++Y)
            {
                for (int X = 0; X < Size - 1; ++X)
                {
                    const bool C = Modules[Y * Size + X];
                    if (C == Modules[Y * Size + X + 1] && C == Modules[(Y + 1) * Size + X] &&
                        C == Modules[(Y + 1) * Size + X + 1])
                    {
                        Result += N2;
                    }
                }
            }

            int Dark = 0;
            for (bool M : Modules)
            {
                if (M) ++Dark;
            }
            const int Total = Size * Size;
            const int K = (std::abs(Dark * 20 - Total * 10) + Total - 1) / Total - 1;
            Result += K * N4;
            return Result;
        }

        static int CountFinderLike(const int H[7])
        {
            const int N = H[1];
            const bool bCore = N > 0 && H[2] == N && H[3] == N * 3 && H[4] == N && H[5] == N;
            return (bCore && H[0] >= N * 4 && H[6] >= N ? 1 : 0)
                 + (bCore && H[6] >= N * 4 && H[0] >= N ? 1 : 0);
        }

        int TerminateAndCount(bool bRunColor, int RunLen, int H[7]) const
        {
            if (bRunColor)
            {
                AddHistory(RunLen, H);
                RunLen = 0;
            }
            RunLen += Size; // the light quiet zone after the last module
            AddHistory(RunLen, H);
            return CountFinderLike(H);
        }

        void AddHistory(int RunLen, int H[7]) const
        {
            if (H[0] == 0)
            {
                RunLen += Size; // the light quiet zone before the first module
            }
            for (int I = 6; I >= 1; --I)
            {
                H[I] = H[I - 1];
            }
            H[0] = RunLen;
        }

        // ---------------- Reed-Solomon over GF(2^8 / 0x11D) ----------------

        static std::vector<uint8_t> ReedSolomonDivisor(int Degree)
        {
            std::vector<uint8_t> Result(Degree, 0);
            Result[Degree - 1] = 1;
            int Root = 1;
            for (int I = 0; I < Degree; ++I)
            {
                for (int J = 0; J < Degree; ++J)
                {
                    Result[J] = GfMultiply(Result[J], Root);
                    if (J + 1 < Degree)
                    {
                        Result[J] ^= Result[J + 1];
                    }
                }
                Root = GfMultiply(Root, 0x02);
            }
            return Result;
        }

        static std::vector<uint8_t> ReedSolomonRemainder(const std::vector<uint8_t>& Data, const std::vector<uint8_t>& Divisor)
        {
            std::vector<uint8_t> Result(Divisor.size(), 0);
            for (uint8_t B : Data)
            {
                const int Factor = B ^ Result[0];
                for (size_t I = 0; I + 1 < Result.size(); ++I)
                {
                    Result[I] = Result[I + 1];
                }
                Result[Result.size() - 1] = 0;
                for (size_t I = 0; I < Result.size(); ++I)
                {
                    Result[I] ^= GfMultiply(Divisor[I], Factor);
                }
            }
            return Result;
        }

        static uint8_t GfMultiply(int X, int Y)
        {
            int Z = 0;
            for (int I = 7; I >= 0; --I)
            {
                Z = (Z << 1) ^ ((Z >> 7) * 0x11D);
                Z ^= ((Y >> I) & 1) * X;
            }
            return static_cast<uint8_t>(Z);
        }

        // ---------------- capacity tables ----------------

        static int NumRawDataModules(int Ver)
        {
            int Result = (16 * Ver + 128) * Ver + 64;
            if (Ver >= 2)
            {
                const int NumAlign = Ver / 7 + 2;
                Result -= (25 * NumAlign - 10) * NumAlign - 55;
                if (Ver >= 7)
                {
                    Result -= 36;
                }
            }
            return Result;
        }

        static int NumDataCodewords(int Ver, EEcc InEcc)
        {
            return NumRawDataModules(Ver) / 8
                 - EccCodewordsPerBlock(InEcc, Ver) * NumErrorCorrectionBlocks(InEcc, Ver);
        }

        static int FormatBits(EEcc InEcc)
        {
            switch (InEcc)
            {
            case EEcc::Low: return 1;
            case EEcc::Medium: return 0;
            case EEcc::Quartile: return 3;
            default: return 2;
            }
        }

        static bool Bit(int X, int I)
        {
            return ((X >> I) & 1) != 0;
        }

        static int EccCodewordsPerBlock(EEcc InEcc, int Ver)
        {
            static const int8_t Table[4][41] = {
                { -1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
                { -1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28 },
                { -1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
                { -1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
            };
            return Table[static_cast<int>(InEcc)][Ver];
        }

        static int NumErrorCorrectionBlocks(EEcc InEcc, int Ver)
        {
            static const int8_t Table[4][41] = {
                { -1, 1, 1, 1, 1, 1, 2, 2, 2, 2,  4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25 },
                { -1, 1, 1, 1, 2, 2, 4, 4, 4, 5,  5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49 },
                { -1, 1, 1, 2, 2, 4, 4, 6, 6, 8,  8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68 },
                { -1, 1, 1, 2, 4, 4, 4, 5, 6, 8,  8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81 },
            };
            return Table[static_cast<int>(InEcc)][Ver];
        }

        static void AppendBits(std::vector<bool>& Bits, int Value, int Length)
        {
            for (int I = Length - 1; I >= 0; --I)
            {
                Bits.push_back(((Value >> I) & 1) != 0);
            }
        }
    };
}
