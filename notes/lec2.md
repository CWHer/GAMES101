## 向量与线性代数

**Graphics’ Dependencies**

- Basic mathematics

  Linear algebra, calculus, statistics

- Basic physics

  Optics, Mechanics

- Misc

  Signal processing, Numerical analysis



### 向量

向量（默认为<u>列向量</u>）

符号$\boldsymbol{a}$，有方向和长度，平移不变

长度：$\left \| \boldsymbol{a} \right \| $

单位向量：$\boldsymbol{\hat a}=\boldsymbol{a} /\left \| \boldsymbol{a} \right \| $，表示方向不关心长度

向量加法

向量乘法

- Dot (scalar) product

  - 用来求向量夹角

  - 求一个向量在另一个向量的投影
    $$
    \vec b_{\perp} =\left \| b \right \| \cos \theta \cdot \vec a \\
    \vec b_{\parallel}=\vec b - \vec b_{\perp}
    $$
    
  - 计算两个方向接近程度
  
  - 判断前后
  
- Cross (vector) product
  $$
  \vec a\times \vec b=A^{\star}b=
  \begin{pmatrix}
  0  & -z_a & y_a \\
  z_a  & 0 & -x_a \\
   -y_a & x_a & 0
  \end{pmatrix}
  \vec b
  $$
  $A^{\star}$ is dual matrix of $\vec a$

  - 用于建立坐标系

  - 判断左/右

  - 判断内/外

    点和三角形的关系：假设$\Delta ABC$逆时针，P点分别在$\overrightarrow {AB},\overrightarrow {BC},\overrightarrow {CA}$左侧（可以推广到凸包）

  

**正交坐标系**
$$
\left \| \vec u \right \| =\left \| \vec v \right \| =\left \| \vec w \right \| =1 \\
\vec u \cdot \vec v=\cdots=0 \\
\vec w=\vec u \times \vec v \ (\text{right-handed}) \\
\vec{p}=(\vec{p} \cdot \vec{u}) \vec{u}+(\vec{p} \cdot \vec{v}) \vec{v}+(\vec{p} \cdot \vec{w}) \vec{w}
$$


### 矩阵

$A\vec x$表示变换 (transformation)，没有交换律



