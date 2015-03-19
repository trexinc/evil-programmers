# Using Mingw GCC #

Read http://mingw.org/wiki/Getting_Started and download **mingw-get**.

Then run:
`mingw-get install gcc g++ mingw32-make mingw32-utils msys-core msys-m4 msys-bison msys-coreutils msys-diffutils msys-make`

Don't forget to run **msys.bat** and from there:
`/postinstall/pi.sh`

Don't forget to add **mingw\bin** and **mingw\msys\1.0\bin** to **%PATH%**.

From http://sourceforge.net/projects/re2c download: **re2c-0.13.5-bin.zip**
<br>re2c-0.13.5-bin.zip needs to be extracted to <b>mingw/bin</b>

Now, to compile the plugins you need to use the following command:<br>
<blockquote><code>mingw32-make -f makefile COMMON=../path_to_common_from_far_svn</code>
some plugins might need:<br>
<code>mingw32-make -f makefile COMMON=../path_to_common_from_far_svn/unicode</code>
and some plugins might need:<br>
<code>mingw32-make -f makefile_gcc FARDEV=../path_to_far_svn_root</code></blockquote>

Some plugins might require the CRT (<b>common/CRT</b>) lib from Far repository, it can be built as follows:<br>
<blockquote><code>mingw32-make -f makefile_lib_gcc</code></blockquote>

For x64 builds (not all plugins support it) use MingW64 from <a href='http://tdm-gcc.tdragon.net'>http://tdm-gcc.tdragon.net</a> (and msys from above) and add <b>DIRBIT=64</b> to all make commands.