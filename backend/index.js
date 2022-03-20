import express from 'express'
import fileupload from 'express-fileupload';
import util from 'util';
import { exec as cp_exec } from 'child_process';

const BINARY = './engine'

const app = express()
app.use(fileupload({
	useTempFiles: true,
	tempFileDir: '/tmp/'
}));

const exec = util.promisify(cp_exec);

app.get('/', (req, res) => {
	res.send('Backend express')
})

app.post('/api/decode', (req, res) => {
	if (!req.files?.input) {
		res.send('Aucun fichier donné');
		return;
	}

	exec(`${BINARY} --decode /tmp/${req.files.input.name}`)
		.then(result => {
			res.send(result);
		}).catch(e => {
			res.status(500).send(`Une erreur s\'est produite : ${e}`);
		});
});

app.listen(3000, () => console.log('Backend started'))
