for l in ./6.5.0/gcc_64/lib/*.so; do
  echo $l; objdump -p $l | grep NEEDED | sed "s/^/\t"/;
 done | grep xcb | awk '{print $2}' \
| while read lib; do echo $lib; dlocate $lib; done