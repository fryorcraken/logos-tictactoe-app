{
  description = "Multiplayer tic-tac-toe — single Logos C++ UI module, QML view via QQuickWidget";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder/tutorial-v1";

    # Pin: head of `tutorial-v1-compat` on logos-delivery-module. Forked from
    # tag 1.0.0 (what basecamp v0.1.1 ships — RPCs return plain `bool`,
    # pre-`LogosResult`) with minimal module-builder packaging on top. Must
    # stay pinned to a commit on that branch so the wire format of the typed
    # SDK headers we generate matches basecamp's bundled delivery_module.
    # See: https://github.com/logos-co/logos-delivery-module/pull/23
    delivery_module.url = "github:logos-co/logos-delivery-module/1fde1566291fe062b98255003b9166b0261c6081";

    # Force delivery_module's transitive `logos-module-builder` to follow our
    # tutorial-v1 pin. Without this, delivery_module drags in its own master-
    # branch module-builder (incompatible with basecamp v0.1.1's bundled
    # delivery_module wire format) as a second entry in flake.lock, and that
    # extra entry silently wins in parts of the build graph.
    # Tracking: https://github.com/logos-co/logos-module-builder/issues/83
    delivery_module.inputs.logos-module-builder.follows = "logos-module-builder";
  };

  outputs = inputs@{ logos-module-builder, delivery_module, ... }:
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
