{
  lib,
  stdenv,
  cmake,
  gepetto-viewer-corba,
  hpp-manipulation-corba,
  pkg-config,
  libsForQt5,
}:

stdenv.mkDerivation {
  pname = "hpp-gui";
  version = "5.0.0";

  src = lib.fileset.toSource {
    root = ./.;
    fileset = lib.fileset.unions [
      ./CMakeLists.txt
      ./doc
      ./etc
      ./package.xml
      ./plugins
      ./pyplugins
      ./res
    ];
  };

  strictDeps = true;

  nativeBuildInputs = [
    cmake
    libsForQt5.wrapQtAppsHook
    pkg-config
  ];
  buildInputs = [ libsForQt5.qtbase ];
  propagatedBuildInputs = [
    gepetto-viewer-corba
    hpp-manipulation-corba
  ];

  doCheck = true;

  meta = {
    description = "Qt based GUI for HPP project";
    homepage = "https://github.com/humanoid-path-planner/hpp-gui";
    license = lib.licenses.bsd2;
    maintainers = [ lib.maintainers.nim65s ];
  };
}
