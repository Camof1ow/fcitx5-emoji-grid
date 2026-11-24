{
  description = "fcitx5 이모지 그리드 선택기 (Super+;)";

  # 시스템(/etc/nixos/flake.lock)과 같은 nixpkgs. 돌고 있는 fcitx5 와 같은 빌드에 링크해야 애드온이 로드된다.
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/c59305bab2065cfecc4944690d9eedbb56f3a9fa";

  outputs = { self, nixpkgs }:
    let
      pkgs = nixpkgs.legacyPackages.aarch64-linux;
    in {
      packages.aarch64-linux.default = pkgs.stdenv.mkDerivation {
        pname = "fcitx5-emoji-grid";
        version = "0.1";
        src = ./.;

        nativeBuildInputs = with pkgs; [ cmake pkg-config wrapGAppsHook4 gobject-introspection ];
        buildInputs = with pkgs; [
          fcitx5
          gtk4
          gtk4-layer-shell
          (python3.withPackages (ps: [ ps.pygobject3 ]))
        ];

        # gtk4-layer-shell 은 libwayland 보다 먼저 로드돼야 한다(Python 에서는 LD_PRELOAD).
        preFixup = ''
          gappsWrapperArgs+=(
            --set LD_PRELOAD ${pkgs.gtk4-layer-shell}/lib/libgtk4-layer-shell.so
            --set EMOJI_TEST ${pkgs.unicode-emoji}/share/unicode/emoji/emoji-test.txt
          )
        '';
      };
    };
}
