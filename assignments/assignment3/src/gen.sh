#！/bin/bash


make clean
make -j4
./Rasterizer normal.png normal
./Rasterizer phong.png phong
./Rasterizer texture.png texture
./Rasterizer bump.png bump
./Rasterizer displacement.png displacement
