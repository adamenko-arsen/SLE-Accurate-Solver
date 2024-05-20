#!/usr/bin/env bash

TMP=Temp
SRC=Sources
OBJ=Objects
EXE="Executable/Розв'язувач СЛАР.exe"

CPP=g++
CPPFLAGS='-std=c++23 -Wall -Wextra'
CPPOPT=-O0
CPPLIBS='-ISources $(pkg-config --cflags --libs gtkmm-3.0)'
CPPLINK=-s

LINK=g++
LINKFLAGS=''
LINKRPATH='-Wl,-rpath,/mingw32/bin:/mingw64/bin'

CPPLINT=cppcheck
CPPLINTFLAGS='--std=c++23 --enable=style,warning,performance,portability,missingInclude --inconclusive'

build() {
    # create the object file structure excactly same as the source's one
    #find "$SOURCE" 

    find "$SRC" -type f -iname "*.cpp" | while read -r cpp;
    do
        out="$OBJ/$(echo "$cpp" | sed "s!^$SRC\/!!" | sed 's/.cpp$/.o/')"

        # if there is no such object file still, then do build the file in any way
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

        # the build command template for running
        cmd="\"$CPP\" -c $CPPLINK $CPPOPT $CPPFLAGS \"$cpp\" -o \"$out\" $CPPLIBS"

        # output a build command and run it
        echo "$cmd"
        bash -c "$cmd"
    done

    # output a build command and run it.
    # find all object files recursively
    cmd="\"$LINK\" $LINKFLAGS $LINKRPATH $(find "$OBJ" -iname '*.o' | xargs) -o \"$EXE\" $CPPLIBS"

    echo "$cmd"
    bash -c "$cmd"
}

touch() {
    find "$SRC" -iname '*.cpp' -exec touch {} \;
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
 - make touch   -- update the modify date of all sources

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
    find "$SRC" \( -iname '*.cpp' -o -iname '*.hpp' \) -exec du -b {} \; > "$TMP/src-size"
    find "$SRC" \( -iname '*.cpp' -o -iname '*.hpp' \) -exec wc -l {} \; | awk '{ print $1 }' > "$TMP/src-line"

    s="$(paste "$TMP/src-line" "$TMP/src-size" | awk '{ print $2 "\t" $1 "\t" $3 }' | sort -rnk1)"

    printf '[size]\t[lines]\t[file]\n%s\n' "$s"
}

case "$1" in
    build)
        build
    ;;
    run)
        run "${@:2}"
    ;;
    touch)
        touch
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
