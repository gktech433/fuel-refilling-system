import express from 'express';
import fs from 'fs';
import path from 'path';

const app = express();
const port = 3000;

app.post('/upload', function (req, res, next) {
  req.pipe(fs.createWriteStream('./uploadFile'));
  req.on('end', next);
});

app.listen(3000);