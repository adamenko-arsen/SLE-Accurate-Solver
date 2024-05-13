#!/usr/bin/env bash

SRC='Sources'
OBJ='Objects'
EXE="Executable/Розв'язувач СЛАР.exe"

EXE="Executable/Розв'язувач СЛАР.exe"

CPP='g++'
CPPFLAGS='-std=c++23 -Wall -Wextra'
CPPOPT='-O0'
CPPLIBS='$(pkg-config --cflags --libs gtkmm-3.0)'
CPPLINK='-s'

LINK='g++'
LINKFLAGS=''

CPPLINT='cppcheck'
CPPLINTFLAGS='--std=c++23 --enable=style,warning,performance,portability,missingInclude --inconclusive'

build() {
    find "$SRC" -type f -iname "*.cpp" | while read -r cpp;
    do
        echo "$cpp"

        out="$OBJ/$(basename "${cpp%.cpp}.o")"

        if [ -e "$out" ];
        then
            hpp="${cpp%.cpp}.hpp"

            cpp_ch="$(stat -c '%Y' "$cpp")"
            out_ch="$(stat -c '%Y' "$out")"

            is_changed=0

            if (( "$cpp_ch" > "$out_ch" ));
            then
                is_changed=1
            fi

            if [ -e "$hpp" ];
            then
                hpp_ch="$(stat -c '%Y' "$hpp")"

                if (( "$hpp_ch" > "$out_ch" ));
                then
                    is_changed=1
                fi
            fi

            if [ "$is_changed" = 0 ];
            then
                continue
            fi
        fi

        cmd="\"$CPP\" -c $CPPLINK $CPPOPT $CPPFLAGS \"$cpp\" -o \"$out\" $CPPLIBS"

        echo "$cmd"
        bash -c "$cmd"
    done

    cmd="\"$LINK\" $LINKFLAGS $OBJ/*.o -o \"$EXE\" $CPPLIBS"

    echo "$cmd"
    bash -c "$cmd"
}

lint() {
    "$CPPLINT" $CPPLINTFLAGS "$SRC"
}

help() {
    cat <<EOF

The list of available options:

 [Build/Run]
 - make build   -- build the most optimized release version
 - make run     -- run the normal way

 [Other]
 - make lint    -- lint the sources
 - make size    -- type the project's size
 - make help    -- type this manual

EOF
}

run() {
    "$EXE" "$@"
}

size() {
    du -sh *
}

case "$1" in
    build)
        build
    ;;
    run)
        run "${@:2}"
    ;;
    size)
        size
    ;;
    lint)
        lint
    ;;
    help)
        help
    ;;
    '')
        help
    ;;
    *)
        echo "No such target as <$1>. Type <help> to print the help manual"
    ;;
esac
