#!/bin/bash

VERSION_REGEX="([0-9]\.[0-9]{2}-[0-9]{6}\.[0-9]{2}\.[0-9])+"
URL=

set_url_platform()
{
  local platform=$1
  URL="https://download.suse.de/updates/SUSE/Updates/SLE-Module-Basesystem/$platform/x86_64/update/x86_64"
}

web_get()
{
  echo downloading "$1"
  if [ -z "$2" ]; then
    wget -q --show-progress --no-check-certificate "$1"
  else
    wget -q --show-progress --no-check-certificate -O "$2" "$1"
  fi

  if [ $? -eq 4 ]; then
    echo Unable to download $1
    exit 1
  fi
}

get_version_from_package_name()
{
  local package=$1
  local version=$(echo "$1" | grep -Eo $VERSION_REGEX)

  echo $version
}

extract_glibc_package_names()
{
  local file=$1
  local interesting_lines=$(grep -Eo "glibc-$VERSION_REGEX\.x86_64.rpm\"" $1)
  local final=""

  for lib in ${interesting_lines}; do
    lib=${lib%?} # Remove last " from string.
    final="$final $lib"
  done

  echo $final
}

download_package_list()
{
  local url="$URL"
  local list_path=$1

  web_get "$url" "$list_path"
}

download_package()
{
  local package=$1
  local url="$URL/$package"

  web_get "$url"
}

parallel_download_packages()
{
  local packages="$*"

  local pids=""

  for package in $packages; do
    local url="$URL/$package"
    # If package already exists, do not bother downloading them again
    if [ ! -f "$package" ]; then
      echo "downloading from $url"
      wget -q --show-progress --no-check-certificate "$url" &
      pid=$!
      pids="$pid $pids"
    else
      echo "Skipping $package because it is already downloaded"
    fi
  done

  # Wait download to finish
  if [ ! -z "$pids" ]; then
    for pid in $pids; do
      wait ${pid}
    done
  fi
}

extract_libs_from_package()
{
  local package=$1
  local version=$(get_version_from_package_name $package)
  mkdir -p $version

  cp $package $version/$package
  cd $version
    rpm2cpio $package | cpio -idmv --quiet

    cp -r lib64/* .

    # delete anything we don't need.
    rm -rf etc lib64 sbin usr var
    rm -f *.rpm
  cd ..
}


main()
{
  set_url_platform "15-SP4"
  download_package_list "/tmp/suse_package_list.html"
  local names=$(extract_glibc_package_names "/tmp/suse_package_list.html")

  parallel_download_packages "$names"

  for package in $names; do
    #download_package_for_platform "15-SP4" "$package"
    extract_libs_from_package "$package"
  done

  rm -f *.rpm
  echo "Done."
}

main
