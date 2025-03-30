import express from 'express';
import fs from 'fs';
import path from 'path';
import { v4 as uuidv4 } from 'uuid';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const port = 3000;

// Middleware to handle raw binary data (octet-stream)
app.use(express.raw({ type: 'application/octet-stream', limit: '50mb' })); // Adjust limit as needed

app.post('/np/upload', (req, res) => {
  try {
    if (!req.body || req.body.length === 0) {
      return res.status(400).json({ error: 'No file uploaded' });
    }

    const fileName = `${uuidv4()}.jpg`;
    const uploadDir = path.join(__dirname, 'uploads');

    // Ensure upload directory exists
    if (!fs.existsSync(uploadDir)) {
      fs.mkdirSync(uploadDir, { recursive: true });
    }

    const filePath = path.join(uploadDir, fileName);

    // Write the received binary data correctly
    fs.writeFileSync(filePath, Buffer.from(req.body));

    res.json({ message: 'File uploaded successfully', filePath });
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Failed to save file' });
  }
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
