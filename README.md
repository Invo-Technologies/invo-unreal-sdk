# Invo Unreal Engine Plugin

This project contains the Invo plugin for Unreal Engine 5.2, which was built from source.

## Requirements

- Unreal Engine 5.2 built from source
- Windows, macOS, or Linux operating system

## Building the Project

### Windows

1. Ensure you have Unreal Engine 5.2 built from source and installed on your machine.

2. Clone this repository to your local machine using Git.

3. Navigate to the project directory and right-click on the `.uproject` file, then select `Generate Visual Studio project files`.

4. Open the newly created `.sln` file in Visual Studio.

5. In Visual Studio, select `Build > Build Solution`.

### macOS

1. Ensure you have Unreal Engine 5.2 built from source and installed on your machine.

2. Clone this repository to your local machine using Git.

3. Navigate to the project directory and open the Terminal.

4. In the Terminal, run the command `./MyGame.command` to generate Xcode project files.

5. Open the newly created `.xcworkspace` file in Xcode.

6. In Xcode, select `Product > Build`.

### Linux

(Provide similar steps for Linux, if applicable)

## Using the Invo Plugin

(Describe how to use the Invo plugin in the Unreal Engine project here)

## Contributing

(If you're open to contributions, provide instructions on how others can contribute to the project)

## License

(Provide information about the project's license, if any)

## Installation and Setup

### Step 1: Installing the Plugin

1. **Create a Plugins Folder:** Navigate to the root directory of your Unreal Engine project. Inside your game project directory, create a new folder and name it `Plugins` if it doesn’t already exist.

2. **Add the Invo Plugin:** Drag and drop the `Invo` folder (containing the plugin files) into the `Plugins` folder. 



## Setting Up Secret Key

The Invo plugin requires a secret key to function properly. To set up the secret key:

1. Create a `Secrets.ini` file in the `Config` directory of your project.

2. Open the `Secrets.ini` file in a text editor.

3. Add the following lines to the file:

   ```ini
   [/Script/Invo.UInvoFunctions]
   SecretKey=YourActualSecretKey
   AuthCodeKey=YourAuthCodeKey
   ```

   Replace `YourActualSecretKey` with your actual secret key. (optional) and 
   Replace `AuthCodeKey` with your actual autcode key you get from the OurInvo Dashboard.  



5. Save and close the `Secrets.ini` file.

The `Secrets.ini` file should not be included in your version control system. Ensure that it's listed in your `.gitignore` file to prevent it from being committed to Git.
