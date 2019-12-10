# NavMesh
Este proyecto es una implementación de un generador de NavMeshes. Consiste de una biblioteca que realiza el cálculo del mesh de navegación y de un editor gráfico para crearlos. Para mayor información sobre este proyecto ver <a href="http://e-rojas-fredini.blogspot.com.ar/2013/11/como-generar-un-navmesh-2d.html" target="_blank">esto</a>.

<img src="http://2.bp.blogspot.com/-v40ezRDULHE/UpRR1yZfv9I/AAAAAAAAAGE/NGZ1v1z0qMI/s1600/NavMeshWalkableObstacle.PNG" width="816"/>

<img src="http://1.bp.blogspot.com/-OK-RdqrR9Ng/UpRR139OJuI/AAAAAAAAAGA/MYw5Y0HY1Pc/s1600/ResultNavMesh.PNG" width="816"/>

## Tabla de Contenidos

- [NavMesh](#navmesh)
  - [Tabla de Contenidos](#tabla-de-contenidos)
  - [Dependencias](#dependencias)
  - [Building](#building)
  - [Contacto](#contacto)
  - [Licencia](#licencia)

## Dependencias

El código sólo utiliza [SFML](https://www.sfml-dev.org/index.php) v2.5.1, [clipper](https://github.com/eppz/Clipper) v5.1.6 y [polypartition](https://github.com/ivanfratric/polypartition).

## Building

Para compilar el proyecto, primero se debe crear la solución con [CMake](https://cmake.org/). Actualmente se usa **CMake v3.16**. Seguir los siguientes pasos:

1. **SFML**: Inicializar el sub-modulo de SFML y compilar el proyecto utilizando CMake. Usamos esta configuración:
   * BUILD_SHARED_LIBS -> Off.
   * SFML_INSTALL_PKGCONFIG_FILES -> On. Es necesario para encontrar el paquete de SFML.
2. **Crear proyecto y compilar**: Crear el proyecto utilizando CMake y compilar. Se puede usar este comando cuando se está parado en el root del repo:

```bash
cmake -S . -B build/
```

Si se quiere utilizar una interfaz gráfica se puede usar **cmake-gui** en Windows. Por otro lado una interfaz del shell más amigable es con el comando:

```bash
ccmake -S . -B build/
```

Actualmente, estamos usando **Visual Studio 2019**.

## Contacto

* Mail: erojasfredini@gmail.com
* Twitter: [https://twitter.com/erojasfredini](https://twitter.com/erojasfredini)

## Licencia
[GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html) (LGPLv3).
