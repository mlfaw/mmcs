
### mmcs - my media categories suite... a WIP media viewer with a booru-style media tagging-system to replace [Hydrus](https://github.com/hydrusnetwork/hydrus) for myself
- Very much a WIP. Messing around with GUI code for Win32 and Direct2D at the moment

Examples of tags include `artist:vincent_van_gogh` (or even `vincent_van_gogh_(artist)`) or `baseball_bat` which can be used to filter/query media in a database.

Reasons to make MMCS:
- Image viewing software taking seconds to open images with an SSD is ridiculous
- Hydrus is a resource-hog. Some notes from Windows:
  - Python 2 adds a lot of overhead. GTK+ on Windows adds a lot of overhead.
  - On a freshly started instance with a clean database 600+ MBs of RAM / Private Bytes are used.
  - An extracted build is 200+~ MBs with 30 MBs of executables that can be deduplicated and 90~ MBs of FFMPEG & CV2 binaries.
  - **Hydrus is still great though. I appreciate what the developer has created very much**
- Media importing/hashing can be sped up.
- I want to make it my own.

Platform support priority notes:
1. is Windows 10 (completely updated). The long-path limit can be removed too in Windows 10... God bless...
2. is Windows 7... Direct2D and really every other API is supported by Windows 7 IF the "Platform Update for Windows 7" is installed... which it should be.
3. is Linux & X11. Might want a GUI toolkit for this... GTK+?
4. is BSDs/Unices using X11. This should mostly come from the Linux/X11 port. There's a few annoying things such as openbsd not having a /proc/self/exe (to readlink() & grab binary path to determine if in portable mode)
5. is Linux & Wayland. This can come from a GUI toolkit easily. GTK+ & Qt both have Wayland backends.
6. is Windows 10 and UWP. Some Windows 10 versions prevent applications to be installed from outside the MS Store....
7. macOS
8. is NOT Windows 8.1/8/Vista/XP. They need to die.

Building:
- For Windows: `.\polyglot_build.bat` (Powershell) or `polyglot_build.bat` (CMD Prompt)
- For Linux: `./polyglot_build.bat`

List of things to make/add/use:
- Media decoders:
  - jpg - libjpeg-turbo? 
  - png - libpng?
  - gif
  - webm / vp8 / vp9
  - mkv
  - mp4
  - bmp
  - tga
  - Audia decoders?
    - mp3
    - ogg
    - aac
    - wav
    - flac
- Database(s).
  - SQLite. Whole tag and media databases?
  - PostgreSQL or MySQL for more performant operations? There's a lot of tags...
- Hash functions
  - (Note: Read chunk into buffer and use separate threads to digest in parallel)
  - MD5: Many boorus use this, so this would help synchronize a tag database.
  - SHA256: Used by Hydrus. Would be simpler to import Hydrus databases by using this.
  - SHA512: 50% faster than SHA256 on x64 architectures
  - insert other fast hash functions since large files will be hashed...
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

Currently used libraries:
- SQLite 3.25.3 - statically linked
  - `sqlite3.c` and `sqlite3.h` are added to `mmcs_sources` in `CMakelists.txt`. Simple.
- LibreSSL 2.8.2 - statically linked
  - Built by `polyglot_build.bat` before the MMCS CMake superbuild is.
  - Custom `FindOpenSSL.cmake` file used to link to libcurl & mmcs without modifying an external project's files.
- libcurl 7.62.0 - statically linked
  - Uses LibreSSL through modified CMAKE_MODULE_PATH to use custom `FindOpenSSL.cmake`
  - Replace with another HTTP parser/library wrapped within libtls (HTTPS) / raw sockets (HTTP)?

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
