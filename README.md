
### mmcs - my media categories suite... a WIP media viewer with a booru-style media tagging-system to replace [Hydrus](https://github.com/hydrusnetwork/hydrus) for myself
- Very much a WIP. Messing around with GUI code for Win32 and Direct2D at the moment

Examples of tags include `artist:vincent_van_gogh` (or even `vincent_van_gogh_(artist)`) or `baseball_bat` which can be used to filter/query media in a database.

Reasons to make MMCS:
- Image viewing software taking seconds to open images with an SSD is ridiculous.
- Hydrus is a resource-hog. Some notes from Windows:
  - Python 2 adds a lot of overhead. GTK+ on Windows adds a lot of overhead.
  - On a freshly started instance with a clean database 600+ MBs of RAM / Private Bytes are used.
  - An extracted build is 200+~ MBs with 30 MBs of executables that can be deduplicated and 90~ MBs of FFMPEG & CV2 binaries.
  - **Hydrus is still great though. I appreciate what the developer has created very much**
- Media importing/hashing can be sped up.
- I want to make it my own.

Platform support:
- Windows 10 is the main platform. Windows 7 will be supported too.
- Linux with X11 and Wayland will be supported eventually likely through GTK+.
- BSDs supported will follow the same GUI code as Linux.
- Maybe support for Microsoft's UWP platform which is used on the Windows (10) Store.
- macOS support is far, far away, if ever...

Building:
- Run `./polyglot_build.bat` in a terminal, Powershell, or Command Prompt.

Dependencies:
- `thirdparty/hash_functions/` needs to be the following repo https://github.com/mlfaw/hash_functions
- `thirdparty/sqlite-amalgamation/` needs to be the latest SQLite amalgamation https://www.sqlite.org/download.html
- `thirdparty/stb_image.h` is from https://github.com/nothings/stb
- Disabled libraries ATM:
  - LibreSSL - statically linked
    - Built by `polyglot_build.bat` before the MMCS CMake superbuild is.
    - Custom `FindOpenSSL.cmake` file used to link to libcurl & mmcs without modifying an external project's files.
  - libcurl - statically linked
    - Uses LibreSSL through modified CMAKE_MODULE_PATH to use custom `FindOpenSSL.cmake`
    - Replace with another HTTP parser/library wrapped within libtls (HTTPS) / raw sockets (HTTP)?

List of things to make/add/use:
- Media decoders:
  - jpg - libjpeg-turbo
  - png - libpng
  - gif - giflib
  - webm / vp8 / vp9 - libvpx(?) / libwebm
  - mkv
  - mp4
  - bmp
  - tga
  - Audia decoders?
    - mp3
    - ogg
    - aac
    - wav
    - flac - libFLAC
- Database(s).
  - SQLite. Entire tag and media databases?
  - PostgreSQL or MySQL for more performant operations? There's a lot of tags...
- Hash functions
  - (Note: Read chunk into buffer and use separate threads to digest in parallel)
  - MD5: Many boorus use this, so this would help synchronize a tag database.
  - SHA-256: Used by Hydrus. Would be simpler to import Hydrus databases by using this.
    - https://en.wikipedia.org/wiki/Intel_SHA_extensions
- JSON parser and generator. So much uses JSON... 
- HTML parser.
- HTTP, HTTPS, HTTP/2, encryption libs (libressl, openssl, boringssl, GnuTLS, WinAPI)...
- Regex engine? PCRE if so?
- Webextension using Native Messaging.
  - When visiting a page like pixiv it will pass off the content to mmcs
- OAuth(2) for Twitter and other APIs? Bot usage on twitter is pretty restrictive and it might be easier & better to scrape HTML
- JavaScript? (Cloudflare captchas for example?)
- https://docs.microsoft.com/en-us/windows/desktop/ProcThread/thread-pools
- Hydrus-like network rate limiting.
- Multiple profiles for MMCS and multiple account support for sites (cookie management / proxies).
- Support for lossless tags or something of the sort? AKA don't restrict mainly a-zA-Z0-9_-
  - Also, use "namespace:asdf" tags like Hydrus?
- Support for hosting a booru which is accessible via HTTP.
- Support for Hydrus' public tag database.
- Lots of images and previews caching from websites.
- Search across multiple sites with tags.

Misc notes:
- Icon sizes: 16, 20, 24, 32, 40, 48, 64, 96, 128, 256, 512
- Tag relationships? (aka relevant tags)
- Version format like 2018.08.05.13?
- ~~puff.c (small inflator for DEFLATE from zlib/contrib)~~
- ~~APPNOTE.TXT - .ZIP File Format Specification~~
- PhAddLayoutItem(&LayoutManager, GetDlgItem(hwndDlg, IDCANCEL), NULL, PH_ANCHOR_BOTTOM | PH_ANCHOR_RIGHT);
- ListViewSample.cpp
- Search \*chan archive with media hash (base64 of raw md5 hash bytes)
- Create Window in main thread. Open database and load previous session in new thread & pass results to main thread.
- Have the (main) media window be implemented as a child-window with the same dimensions as the largest connected monitor. When the media viewport resizes the media window draws media at a different position and size, but does not need to resize the actual HWND and does not need to recreate the swap chain. \*Monitor resolution changes can trigger media widnow resizes (and swap-chain recreation due to resizes).
    - This approach's effectiveness will known on implementation... and hopefully end my quest for smooth media painting even during window-resizing...
- The media-preview grid SHOULD NOT use new HWNDs for every item. 1000s of results = 1000s of windows. This may mean using a fixed number of preview HWNDs that paint new previews when scrolling. This may also mean using DirectX, Direct2D, OpenGL, or Vulkan to draw the layout and images entirely...
- Figure out how to retrieve the Windows 10 setting for Light or Dark Mode... and obviously react to it... And the accent color too...

Relevant links or projects:
- https://skia.org/
- https://www.cairographics.org/manual/
- https://github.com/Facebook/folly
- https://github.com/Tencent/rapidjson
- https://github.com/hydrusnetwork/hydrus
- https://doujindownloader.com/
- https://github.com/fake-name/xA-Scraper
- https://github.com/fake-name/MangaCMS
- https://github.com/fake-name/AutoTriever - distributed content fetching... nice
- https://waifu2x.me/ - plugin for this?
- https://github.com/tsuruclient/tsuru
- https://github.com/sergeyk/rayleigh
- https://github.com/Nandaka/PixivUtil2
- https://github.com/CuddleBear92/Hydrus-Presets-and-Scripts/issues/56
  - Many links to other downloaders.
- https://github.com/CuddleBear92/Hydrus-Presets-and-Scripts/issues/57
  - Often, images from websites will not be the same image that a person created. This is due to resizing, reencoding, and stripping of metadata. This has links for perceptual hashing instead of file-content hashing.
