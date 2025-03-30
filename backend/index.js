import 'dotenv/config';
import express from 'express';
import { S3Client, PutObjectCommand } from "@aws-sdk/client-s3";
import multer from 'multer';
import { v4 as uuidv4 } from 'uuid';

const app = express();
const port = 3000;

// AWS S3 Configuration
const s3Client = new S3Client();

app.use(express.json({ limit: '10mb' }));

app.post('/np/upload', async (req, res) => {
  try {
    console.log(req.body);
    const contentType = 'image/jpeg';
    const bucketName = process.env.BUCKET_NAME;
    const { image } = req.body; // Expecting base64 image string
    console.log(image)
    if (!image) {
      return res.status(400).json({ error: 'No image provided' });
    }

    // Extract base64 data
    const matches = image.match(/^data:image\/(\w+);base64,(.+)$/);
    if (!matches || matches.length !== 3) {
      return res.status(400).json({ error: 'Invalid base64 image format' });
    }

    const ext = matches[1]; // Extract file extension (e.g., png, jpg)
    const buffer = Buffer.from(image, 'base64');
    const key = `uploads/${uuidv4()}.jpeg`;

    // Upload to S3
    // Create a PutObjectCommand to upload the file to S3
    const command = new PutObjectCommand({
        Bucket: bucketName,
        Key: key, // The file name to use in S3
        Body: buffer,
        ContentType: contentType,
    });

    // Upload the image to S3
    const data = await s3Client.send(command);
    res.json({ message: 'Upload successful' });
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Failed to upload image' });
  }
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
