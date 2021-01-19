# Particle Photon code

The Particle Photon subsystem software named 2020_photon_code:	
The entire '2020_photon_code' folder is a Visual Studio project that uses the Particle Workbench and dependencies to program the Photons (remotely).
## Welcome to the project!

#### ```/src``` folder:  
This is the source folder that contains the firmware files for the project. It should *not* be renamed. 
Anything that is in this folder when you compile your project will be sent to the Particle compile service and compiled into a firmware binary for the Particle device that you have targeted.
The project is set up for Photon v2.0.1.

The main files are included in the `src` folder. The dependencies are specified in the `project.properties` file referenced below.

#### ```.ino``` file:
This file is the firmware that will run as the primary application on the Particle device. It contains a `setup()` and `loop()` function, and is written in C++.

#### ```project.properties``` file:  
This is the file that specifies the name and version number of the libraries that the project depends on. Dependencies are added automatically to the `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

## Adding additional files to the project

#### Projects with multiple sources
If you would like add additional files to your application, they should be added to the `/src` folder. All files in the `/src` folder will be sent to the Particle Cloud to produce a compiled binary.

#### Projects with external libraries
If the project includes a library that has not been registered in the Particle libraries system, you should create a new folder named `/lib/<libraryname>/src` under `/<project dir>` and add the `.h`, `.cpp` & `library.properties` files for your library there.

## Compiling the project

When you're ready to compile the project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in the project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project
- Any libraries stored under `lib/<libraryname>/src`
