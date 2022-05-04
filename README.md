# hpp-gui

[![Pipeline status](https://gitlab.laas.fr/humanoid-path-planner/hpp-gui/badges/master/pipeline.svg)](https://gitlab.laas.fr/humanoid-path-planner/hpp-gui/commits/master)
[![Coverage report](https://gitlab.laas.fr/humanoid-path-planner/hpp-gui/badges/master/coverage.svg?job=doc-coverage)](https://gepettoweb.laas.fr/doc/humanoid-path-planner/hpp-gui/master/coverage/)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/humanoid-path-planner/hpp-gui/master.svg)](https://results.pre-commit.ci/latest/github/humanoid-path-planner/hpp-gui)

**hpp-gui** is a set of plugins to integrate [HPP] inside `gepetto-gui`, which comes with the `gepetto-viewer-corba` package.

The plugin `hppcorbaserverplugin` create an instance of [hpp-corbaserver] at the gui start.

The plugin `hppwidgetsplugin` add a lot of widgets to define a problem.

The plugin `hppmanipulationwidgetsplugin` do the same things as `hppwidgetsplugin` but for the manipulation part of [HPP].

## Installation procedure
There are a few dependencies to be installed before installing *hpp-gui*.

### Dependencies
There are a few required dependencies and several optional ones.
You must install [HPP]. You will find the installation procedure in the [hpp-doc] github page.
You must also install `gepetto-gui` from [gepetto-viewer-corba].

This package depends on:
* [gepetto-viewer-corba] - contains `gepetto-gui`.
* [hpp-corbaserver] - comes with the [HPP] framework.
* [hpp-manipulation-corba] - comes with [HPP] framework.

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

[HPP]:http://projects.laas.fr/gepetto/index.php/Software/Hpp
[hpp-corbaserver]:https://github.com/humanoid-path-planner/hpp-corbaserver
[hpp-manipulation-corba]:https://github.com/humanoid-path-planner/hpp-manipulation-corba
[gepetto-viewer-corba]:https://github.com/humanoid-path-planner/gepetto-viewer-corba
[hpp-doc]:https://github.com/humanoid-path-planner/hpp-doc
[remoteimu]:https://github.com/jmirabel/remoteimu
[hpp-plot]:https://github.com/jmirabel/hpp-plot
