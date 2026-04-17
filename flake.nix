{
  description = "Qt based GUI for HPP project";

  inputs.gepetto.url = "github:gepetto/nix";

  outputs =
    inputs:
    inputs.gepetto.lib.mkFlakoboros inputs (
      { lib, ... }:
      {
        overrideAttrs.hpp-gui = {
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
        };
      }
    );
}
