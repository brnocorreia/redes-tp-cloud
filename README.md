# **Projeto Cliente-Servidor com Sockets TCP**

## 📖 **Visão Geral**

Este projeto implementa uma comunicação cliente-servidor utilizando **sockets TCP** e as bibliotecas de rede em **C++**. O objetivo é criar uma aplicação que transmite arquivos de forma segura, garantindo que comandos especiais como o **"bye"** (utilizado para terminar a conexão) não sejam confundidos com nomes de arquivos. Para isso, é aplicada a técnica de **byte stuffing**, onde sequências especiais são inseridas para garantir que dados sensíveis (como o nome de arquivos) sejam transmitidos de forma correta.

## 🧑‍💻 **Estrutura do Projeto**

Este projeto é composto por dois programas principais:

- **Cliente**: Envia dados para o servidor, incluindo nomes de arquivos de um diretório especificado.
- **Servidor**: Recebe os nomes dos arquivos e armazena-os em um arquivo de log, tratando corretamente a sequência "bye" com byte stuffing.

## ⚙️ **Tecnologias Usadas**

- **Linguagem**: C++
- **Sockets**: TCP (IPv4 e IPv6)
- **Biblioteca de Sockets**: BSD sockets
- **Byte Stuffing**: Técnica usada para evitar confusão com sequências especiais, como o comando de **"bye"** [TO BE IMPLEMENTED].

## 🧑‍💻 **Requisitos de Execução**

- **Compilador**: Para executar o projeto, é necessário ter o **GNU Compiler Collection (GCC)** instalado em seu sistema. A versão mínima requerida é **GCC 7.1.0**, sendo a recomendada a versão **GCC 13.2.0**.

## 💡 **Funcionamento do Sistema**

### 1. **Cliente**

O cliente realiza a conexão com o servidor passando os seguintes parâmetros:

- **server_host**: Endereço IP ou nome do servidor (por exemplo, "localhost").
- **server_port**: Porta do servidor.
- **dir_name**: Diretório que contém os arquivos a serem enviados.

#### Fluxo de Comunicação:

1. O cliente estabelece uma conexão com o servidor.
2. Envia uma mensagem **"READY"** para o servidor.
3. Quando o servidor responde com **"READY ACK"**, o cliente envia uma lista com todos os **nomes dos arquivos** encontrados no diretório especificado.
4. Se o nome do arquivo for **"bye"**, a técnica de **byte stuffing** é aplicada, substituindo **"bye"** por uma sequência especial.
5. Após o envio, o cliente envia a mensagem **"bye"** para finalizar a conexão.
6. O cliente mede o tempo total de envio e calcula o tempo gasto para a transmissão.

### 2. **Servidor**

O servidor realiza o seguinte:

1. Escuta a porta fornecida pelo cliente.
2. Após receber a mensagem **"READY"**, responde com **"READY ACK"**.
3. Recebe a lista de arquivos e processa os nomes, armazenando-os em um arquivo de log.
4. Se encontrar a sequência especial **`0x7D 0x01`**, ele a converte de volta para **"bye"**.
5. Quando o cliente enviar **"bye"**, o servidor finaliza o processo, salvando o arquivo de log.

#### **Arquivo de Log**:

O servidor salva os nomes dos arquivos em um arquivo nomeado com o formato: <server_host><dir_name>.txt

### 3. **Generate Tests**

A aplicação conta com um gerador de testes, que gera 64 arquivos .json com nomes aleatórios de tamanho i bytes, valor configurável através de arg no binário executável. Os arquivos são criados na pasta ./tests.

### 4. **Byte Stuffing (ESC-Flag)**

Quando o nome do arquivo for **"bye"**, ele será substituído pela sequência especial **`0x7D 0x01`** (ESC + Flag). Essa técnica é aplicada para garantir que a mensagem de término da conexão não se misture com o nome de arquivos. O servidor, por sua vez, converte a sequência de volta para **"bye"**.

## 🚀 **Instruções para Execução**

### **1. Compilação**

Para compilar o cliente, o servidor e o gerador de testes, use o seguinte comando no terminal:

```bash
g++ client.cpp -o client -std=c++11
g++ server.cpp -o server -std=c++11
g++ gen_tests.cpp -o gen_tests -std=c++11
```

ou, se preferir, use o comando Make:

```bash
make all // Caso queira compilar todos de vez
make client
make server
make tests
```

**Obs.:**
Por padrão, o Makefile usa:

```make
# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17
```

### 2. **Execução**

Para executar o servidor, use o seguinte comando no terminal:

```bash
./server <server_port> <buffer_size>
```

Onde:

- `server_port`: Porta do servidor.
- `buffer_size`: Tamanho do buffer (2^i), onde i é o valor passado.

Por exemplo:

```bash
./server 8080 8
```

Para executar o cliente, use o seguinte comando no terminal:

```bash
./client <server_host> <server_port> <dir_name> <buffer_size>
```

Onde:

- `server_host`: Endereço IP ou nome do servidor (por exemplo, 192.168.1.100).
- `server_port`: Porta do servidor.
- `dir_name`: Diretório que contém os arquivos a serem enviados.
- `buffer_size`: Tamanho do buffer (2^i), onde i é o valor passado.

Por exemplo:

```bash
./client 192.168.1.100 8080 /home/brnocorreia/projects/ufba/redes-tp-cloud 8
```

**IMPORTANTE**
O valor do buffer size precisa ser igual em ambas as aplicações e de preferência também no gerador de testes (esse não necessariamente, já que mesmo que o nome do arquivo não contenha o valor x de bytes total, o buffer enviado ainda é 2^i.

Se preferir, pode executar o comando Make:

```bash
make run-client
make run-server
make run-tests
```

Por padrão, o Makefile usa:

```make
# Default server hostname
SERVER_HOST ?= localhost

# Default server port
SERVER_PORT ?= 8080

# Default client directory
CLIENT_DIR ?= ./tests

# Buffer size
BUFFER_SIZE ?= 8
```

Caso queira mudar, basta passar o valor desse modo, mudando o valor de alguma das variáveis:

```bash
make run-client BUFFER_SIZE=10
```
