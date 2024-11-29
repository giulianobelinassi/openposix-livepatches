#!/bin/sh

echo "libc_GLIBCVER_livepatch1.so" > libc_livepatch1.in
echo "@LIBC" >> libc_livepatch1.in
echo "gnu_get_libc_version:gnu_get_libc_version_lp" >> libc_livepatch1.in

# Read all memcpy functions available in glibc.  We can't know the
# CPU capabilities at this point so we do them all.
readelf -sW /lib64/libc.so.6  | awk '{
  if ($4 == "FUNC" && $8 ~ /^__memcpy/ && $8 !~ /ifunc/ && $8 !~ /chk/)
    print $8":memcpy_lp"
}' >> libc_livepatch1.in
