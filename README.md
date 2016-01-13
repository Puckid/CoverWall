# CoverWall
Qt App that automatically and ramdomly create desktop wallpaper from album covers (or any more or less square pictures)

How to use (on Linux):
1) collect some pictures of the cover's of the best albums (preferably Metal!)
2) put the ones you want to appear in priority on your wallpaper in the folder "./img/best/"
3) put the others in the folder "./img/"
4) start the app "./CoverWall"
5) in the interface, you can select several parameters:
    - width: number of smallest picture in the width
    - height: number of smallest picture in the heignt
    - pixels: size of the smallest picture
    - big: size of the biggest pictures (multiple of smallest)
    - N° big: number of big pictures the app tries to fit (might not be able to)
    - R, G, B: to set the color of blank spaces
    You'll see the changes apply on the matrix. And you can click in the matrix to set parts that'll stay blank (usefull to place icons, you won't be able to find them otherwise)
6) push start and let the magic happens! The resulting picture is saved in "." and should open at the end.
7) As a lot of randomness occurs, you can push the button again and again, it will generate a totally different wallpaper! Again, and again!

Compile instruction (easiest with Qt creator):
1) you need to install Qt5 and OpenCV 3.1
2) Compile!
