# Proxy HTTP
Proxy HTTP para Protocolos de Comunicación.
Instituto Tecnológico de Buenos Aires (ITBA)

## Contenido
Proxy HTTP transparente no bloqueante (intento).

## Probar

 1. Terminal 1 - Proxy corriendo
  `make clean && make`
  `./proxy [-p puerto_http] [-o puerto-de-management] [-t cmd] [-M media-types-transformables] [-l direccion-http] [-L direccion-de-management]`
  Por default, el puerto es 8080
 2. Origin server (temporal)
 `Corriendo en 54.233.152.125:9090`
 3. Parte 3 - Cliente
 Terminal Ejemplo: `curl -v -x http://<ip_proxy>:<puerto_proxy> -L http://<ip_server>:<puerto:server>`

 4. Parte 4 - Cliente management
  Terminal ejemplo: `./config_client 127.0.0.1 9090 -h`

### Integrantes
 - Biagini, Martín
 - Cortés Rodríguez, Kevin
 - Filipic, Joaquin
 - Nielavitzky, Jonatan
