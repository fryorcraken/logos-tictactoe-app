{
  description = "Multiplayer tic-tac-toe — single Logos C++ UI module, QML view via QQuickWidget";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder/tutorial-v1";
  };

  outputs = inputs@{ logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosModule {
      src = ./.;
      configFile = ./metadata.json;
      flakeInputs = inputs;

      # Basecamp looks up UI plugins as `<dir>/<dir>.so` but the
      # `logos_module()` CMake macro produces `<dir>_plugin.so`. Ship both so
      # basecamp can load the plugin. Tracking: logos-co/logos-basecamp#136.
      postInstall = ''
        cp $out/lib/tictactoe_plugin.so $out/lib/tictactoe.so
      '';
    };
}
