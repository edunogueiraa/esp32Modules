const express = require('express');
const { MongoClient } = require('mongodb');
require('dotenv').config();

const app = express();
app.use(express.json());

const client = new MongoClient(process.env.MONGODB_URI, { ignoreUndefined: true }); // ignoreUndefined: campos ausentes não viram "null" no banco
const PORT = 3000;

app.get('/', (req, res) => {
    res.send('API Solar funcionando!');
});

async function iniciarServidor() {
    try {
        await client.connect();
        console.log('Conectado ao MongoDB Atlas!');

        const banco = client.db('solar_db');
        const colecao = banco.collection('readings_solar');

        app.post('/api/solar', async (req, res) => {
            try {
                // Pega cada bloco do JSON enviado pelo ESP32
                const {
                    station_id,
                    timestamp: timestampEnviado,
                    location,
                    status,
                    generation,
                    energy,
                    charging,
                    environment,
                    network,
                    device
                } = req.body;

                // Validação mínima: estação e medições principais do MAX471
                if (
                    typeof station_id !== 'string' || station_id.trim() === '' ||
                    !energy ||
                    !Number.isFinite(energy.voltage_v) ||
                    !Number.isFinite(energy.current_a) ||
                    !Number.isFinite(energy.power_w)
                ) {
                    return res.status(400).json({
                        erro: 'Dados inválidos: envie station_id e energy (voltage_v, current_a, power_w).'
                    });
                }

                // Usa o timestamp do ESP32 (NTP); se faltar ou for inválido, usa a hora do servidor
                let timestamp = new Date(timestampEnviado);
                if (!timestampEnviado || isNaN(timestamp.getTime())) timestamp = new Date();

                const registro = {
                    station_id,
                    timestamp,
                    location,
                    status,
                    generation,
                    energy,
                    charging,
                    environment,
                    network,
                    device,
                    recebidoEm: new Date()
                };

                const resultado = await colecao.insertOne(registro);

                res.status(201).json({
                    mensagem: 'Dados solares salvos!',
                    id: resultado.insertedId
                });
            } catch (erro) {
                console.error('Erro ao salvar:', erro);
                res.status(500).json({ erro: 'Erro ao salvar os dados.' });
            }
        });

        // '0.0.0.0' = aceita conexões de outros dispositivos da rede (o ESP32)
        app.listen(PORT, '0.0.0.0', () => {
            console.log(`API disponível na porta ${PORT}`);
        });
    } catch (erro) {
        console.error('Erro ao conectar ao MongoDB Atlas:', erro.message);
    }
}

iniciarServidor();