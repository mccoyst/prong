#!/bin/sh

ldflags_Darwin='-framework SDL2'
ldflags_Linux='-lSDL2'

progname=$(basename $(pwd))
objects=$(echo *.cxx | sed 's/\.cxx/.o/g')

if [ ! -e build.ninja ]; then
	case $(uname) in
	Darwin)
		ldflags=$ldflags_Darwin;;
	Linux)
		ldflags=$ldflags_Linux;;
	esac

	cat > build.ninja <<EOF
cxxflags = -std=c++11 -Wall -Werror
ldflags = $ldflags

rule cx
 command = clang++ \$cxxflags -c \$in -o \$out
 description = cx \$in

rule ld
 command = clang++ \$ldflags \$in -o \$out
 description = linking \$out

EOF

	for o in $objects; do
		c=$(echo $o | sed 's/\.o$/.cxx/g')
		echo "build $o: cx $c" >> build.ninja
	done

	echo "build $progname: ld $objects" >> build.ninja
fi

exec ninja "$@"
