# solarIntelligent

## Descrição do projeto

O módulo **solarIntelligent** tem o objetivo de fazer a leitura dos parâmetros físicos de uma estação de carregamento solar inteligente para bicicletas elétricas e enviá-los para um servidor MongoDB na nuvem.

### Estrutura de arquivos principais

- **`solarIntelligent.ino`**: base para o microcontrolador, com toda a lógica de leitura dos parâmetros, como tensão, corrente, potência, temperatura, umidade e coordenadas. Também é responsável por elaborar o JSON com todos esses dados.

- **`configs.h`**: configurações básicas para a conexão Wi-Fi local e alguns parâmetros do microcontrolador, como tensão e pinos de montagem. Em `SERVER_URL`, use o IP do computador que roda o servidor (veja o IP atual com o comando `hostname -I`).

- **`server.js`**: servidor responsável por receber os dados do microcontrolador localmente e enviá-los para a plataforma MongoDB Atlas.

- **`.env`**: arquivo que guarda a string de conexão do servidor MongoDB Atlas.

---

## Configuração do servidor no Linux

Para utilizar o servidor, é preciso instalar algumas ferramentas.

Instale o Node.js e o npm:

```bash
sudo apt install nodejs npm
```

Dentro da pasta do projeto, rode:

```bash
npm init -y
npm install express mongodb dotenv
```

Isso cria o `package.json` e a pasta `node_modules` com as três bibliotecas que o `server.js` usa: `express`, `mongodb` e `dotenv`.

Crie o arquivo `.env` na mesma pasta do `server.js`, com a string de conexão do MongoDB Atlas:

```env
MONGODB_URI="mongodb+srv://USUARIO:SENHA@cluster0.xxxxx.mongodb.net/?retryWrites=true&w=majority"
```

Rode o servidor:

```bash
node server.js
```

Se tudo estiver certo, o terminal mostra `Conectado ao MongoDB Atlas!` e `API disponível na porta 3000`.

### Observações importantes

- O ESP32 e o computador que roda o servidor precisam estar na **mesma rede Wi-Fi**.
- No MongoDB Atlas, o IP do computador que roda o servidor precisa estar liberado em **Network Access**; caso contrário, a conexão falha e o servidor não abre a porta 3000.
- Se o ESP32 mostrar `connection refused`, confira se o servidor está rodando, se o IP em `SERVER_URL` está correto e se a porta 3000 está liberada no firewall (`sudo ufw allow 3000/tcp`).
- Nunca envie o arquivo `.env` para o GitHub (adicione-o ao `.gitignore`).
