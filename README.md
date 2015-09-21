# hpp-gui

**hpp-gui** is a graphical interface for robot visualization with plugins to integrate [HPP] - Humanoid Path Planner. The interfaces is based on Qt 4 and OpenSceneGraph 3.2.

The core part is independent of [HPP] and can be extended to other tools via the plugin system.

## Summary
* [Usage](#usage)
  *  [Basic usage](#basic-usage)
  *  [Adding predefined robots and environments](#adding-predefined-robots-and-environments)
  *  [Loading plugins](#loading-plugins)
  *  [For HPP developpers](#for-hpp-developpers)
* [Installation procedure](#installation-procedure)
  * [Dependencies](#dependencies)

## Usage

#### Basic usage
Launch the binary file `hpp-gui` and do as in [this video](http://homepages.laas.fr/jmirabel/raw/videos/hpp-gui-example.mp4).

#### Adding predefined robots and environments
For convenience, robots and environments can be predifined. The configuration files are - from the installation prefix - in `etc/hpp-gui`.

Open `${CMAKE_INSTALL_PREFIX}/etc/hpp-gui/robots.conf` and write:
```
[PR2 - hpp_tutorial]
RobotName=pr2
ModelName=pr2
RootJointType=planar
Package=hpp_tutorial
PackagePath=${CMAKE_INSTALL_PREFIX}/share/hpp_tutorial
URDFSuffix=
SRDFSuffix=_manipulation
MeshDirectory=/opt/ros/hydro/share/

[HRP2]
RobotName=hrp2_14
ModelName=hrp2_14
RootJointType=freeflyer
Package=hrp2_14_description
PackagePath=${CMAKE_INSTALL_PREFIX}/share/hrp2_14_description
URDFSuffix=
SRDFSuffix=
MeshDirectory=${CMAKE_INSTALL_PREFIX}/share/
```

Open `${CMAKE_INSTALL_PREFIX}/etc/hpp-gui/environments.conf` and write:
```
[Kitchen]
RobotName=Kitchen
Package=iai_maps
PackagePath=${CMAKE_INSTALL_PREFIX}/share/iai_maps
URDFFilename=kitchen_area
MeshDirectory=${CMAKE_INSTALL_PREFIX}/share/
```

Note: Do not forget to replace `${CMAKE_INSTALL_PREFIX}` by a relevant path.

#### Loading plugins

Open `${CMAKE_INSTALL_PREFIX}/etc/hpp-gui/settings.conf` and write:
```
[plugins]
libhppcorbaserverplugin.so=true
libremoteimuplugin.so=true
libhppwidgetsplugin.so=true
```

The plugins are looked for in the directory `${CMAKE_INSTALL_PREFIX}/lib/hpp-gui-plugins`

#### For HPP developpers
As [HPP], the *GUI* can be controlled using a python interface. When the *GUI* starts, it launches a server for both [HPP] and the Gepetto Viewer exactly as if you were manually launching the two commands `hppcorbaserver` and `gepetto-viewer-server`. This means that **you can run the same python scripts** and it will work !

When you do so, pay attention to the following points:
- the GUI has no way of knowing when to refresh the list of joints and bodies. **There is a refresh button in the "Joint tree" window**.
- moving the robot in the GUI while the server is processing data can lead to unexpected results, because you are modifying the *current configuration* of HPP when not expected.

## Installation procedure
There are a few dependencies to be installed before installing *hpp-gui*.

### Dependencies
There are a few required dependencies and several optional ones.
#### Core interface
There are only two dependencies:
* Qt 4: `sudo apt-get install qt4-dev-tools libqt4-opengl-dev libqtgui4`.
* [gepetto-viewer-corba].

Optionally, for a better rendering:
* `oxygen-icon-theme`: `sudo apt-get install oxygen-icon-theme`

#### Plugins
You must install [HPP]. You will find the installation procedure in the [hpp-doc] github page.

This package depends on:
* [hpp-corbaserver] - comes with the [HPP] framework.

The following packages are optional:
* [remoteimu] - enables controlling a robot body orientation with an IMU (typically, in a smart phone).

Other plugins can be found in the package [hpp-plot].

### Installation of *hpp-gui*
The installation procedure is a classic `cmake` installation.

```sh
git clone https://github.com/jmirabel/hpp-gui.git
mkdir hpp-gui/build && cd hpp-gui/build
cmake ..
make install
```

## Version
0.1

## Development

Want to contribute? Great!
See the ToDo's list below and use the github pull request mechanism.

## Todo's
* Display roadmap;
* Create constraints.

[HPP]:http://projects.laas.fr/gepetto/index.php/Software/Hpp
[hpp-corbaserver]:https://github.com/humanoid-path-planner/hpp-corbaserver
[gepetto-viewer]:https://github.com/humanoid-path-planner/gepetto-viewer
[gepetto-viewer-corba]:https://github.com/humanoid-path-planner/gepetto-viewer-corba
[hpp-doc]:https://github.com/humanoid-path-planner/hpp-doc
[remoteimu]:https://github.com/jmirabel/remoteimu
[hpp-plot]:https://github.com/jmirabel/hpp-plot
