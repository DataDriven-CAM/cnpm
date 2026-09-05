# cnpm
c++ package manager

Managing projects with package.json and also dependencies that don't have a package.json.
Major overhaul of the cnpm project! Changing philosophy to be more lightweight and faster. Much faster and stable. Scheduled bulk tasks for git, hg, and curl.

Then focus will be on recording relationships between projects and their dependencies. And meta data about the projects themselves.
This package manager very soon will produce a JGF v2 dependency graph & meta.mk file. These will be handed off to the [winnow](https://github.com/DataDriven-CAM/winnow) project. 

When a dependency doesn't have a package.json cnpm will call on winnow early and give it the location and the type of the current build system. Except if a project is determined to be header only then it will be left alone. Otherise winnow determine what equivalent package.json describes the project and makes it for cnpm to install its dependencies.

When all dependencies are installed cnpm will call on winnow to autonomously maintain Makefile's for the main project. Users can opt not to use the winnow features alhough if developed effectively, they're lives should be made much easier. winnow eliminates the need for any other build system!

Coming with winnow is winnow-ui for a web based front end at first displaying what winnow is doing. 

## Start
Need gcc 17.2.0 or later.  For windows  (Rev5, Built by MSYS2 project) 11.2.0

```
bootstrap.sh
```
or on windows the powershell 7
```
bootstrap.ps1
```
After initial build , this project can build itself though eventually I hope to distribute to
linux and mac (maybe windows).

## Repeat build while developing this project

```
make -j3
```

Usage is from help

```
./cnpm -h
```

This will work with any language and is most useful where there are dependent projects without packages.

```
cnpm snap //produces a snapscraft yaml
```


Decided to go in the direction of asynchornous subprocess spawning and going fully lightweight development.

Dependencies are all header only except urlcpp(turning it into header ony) & json-thresher which is statically link. Deployment should be easy for me and a lightweight app for a users.

Only expectations, you the user have curl and git(optionally mercurial if you know you have some dependencies that require it) installed.
