#!/usr/bin/env bash
#
# Script to package an AppImage. Assumes it is being run from the build
# directory.
set -e

# AppImage root
APPDIR=${APPDIR:-"appimage_staging"}

if [ -z "$APPDIR" ]; then
    echo "Error: APPDIR must be a defined environment variable."
    exit 1
fi

# linuxdeploy location
LINUXDEPLOY=${LINUXDEPLOY:-"linuxdeploy.AppImage"}

# Extract tar contents to APPDIR; tar file already contains a top
# level dir, so remove it.
TAR_NAME=$(ls packages/xournalpp-*.tar.gz | head -n 1)
if [[ ! -d "$APPDIR" ]]; then
    tar xf $TAR_NAME --one-top-level="$APPDIR"/usr --strip=1
fi

# Download linuxdeploy if it doesn't exist
if [[ ! -f $LINUXDEPLOY ]]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O "$LINUXDEPLOY"
    chmod +x "$LINUXDEPLOY"
fi

set -x

# Package GTK libraries and dependencies. Some of the following code is from
# https://docs.appimage.org/packaging-guide/manual.html#bundling-gtk-libraries
mkdir -p "$APPDIR"/usr/lib

# glib
glib_schema_dir="/usr/share/glib-2.0/schemas"
mkdir -p "${APPDIR}${glib_schema_dir}"
cp -a "$glib_schema_dir" "${APPDIR}${glib_schema_dir}"
glib-compile-schemas "$glib_schema_dir" --targetdir="$APPDIR"/usr/share/glib-2.0/schemas

# Copy gdk_pixbuf, rsvg
for lib in $(ldconfig -p | awk '/librsvg/ || /libgdk_pixbuf/ { print $4; }'); do
    cp -a "$lib" "$APPDIR"/usr/lib/
    # if symlink, copy actual binary too (note: they are copied to the same folder)
    [[ -L "$lib" ]] && cp -a $(realpath "$lib") "$APPDIR"/usr/lib
done

# Copy GDK-Pixbuf modules and rebuild loader cache
gdk_pixbuf_moduledir=$(pkg-config --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0)
gdk_pixbuf_cache_file=$(pkg-config --variable=gdk_pixbuf_cache_file gdk-pixbuf-2.0)
gdk_pixbuf_libdir_bundle="lib/gdk-pixbuf-2.0"
gdk_pixbuf_cache_file_bundle="$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"/loaders.cache
mkdir -p "$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"
cp -a "$gdk_pixbuf_moduledir" "$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"
cp -a "$gdk_pixbuf_cache_file" "$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"
sed -i -e "s|${gdk_pixbuf_moduledir}/||g" "$gdk_pixbuf_cache_file_bundle"

# Copy Adwaita GTK icon theme (the overall theme is already included in GTK)
mkdir -p "$APPDIR"/usr/share/icons
cp -r /usr/share/icons/Adwaita "$APPDIR"/usr/share/icons

# TODO: bundle libasound and jackd
# but note: https://github.com/AppImage/pkg2appimage/blob/972b51b03c303c96dd89af2c19c14ee75712f6ba/excludelist#L176

set +x

# Generate AppImage
APPRUN=$(dirname $0)/appimage_apprun.sh
./linuxdeploy.AppImage --appdir="$APPDIR" --custom-apprun="$APPRUN" --output appimage
