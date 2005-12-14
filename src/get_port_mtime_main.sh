#!/bin/sh

for dir in `cut -f 2 -d '|' /usr/ports/INDEX-6` ; do
	MTIME1="`./get_port_mtime_main $dir |cut -f 2- -d :`"
	MTIME2="`stat -f '%Sm' -t '%Y.%m.%d %H:%M:%S' $dir/Makefile $dir/pkg-plist $dir/files 2>/dev/null |sort |tail -1`"
	if [ "$MTIME1" != "$MTIME2" ] ; then
		echo "$dir failed"
		echo "$MTIME1"
		echo "$MTIME2"
		false
	else
		echo "$dir ok"
	fi
done

# EOF
