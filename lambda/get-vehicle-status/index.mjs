import 'dotenv/config';
import pg from 'pg';
import fs from 'fs';
import { getVehicleNo } from './get-vehicle-no.mjs';

const { Client } = pg
const client = new Client({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    port: process.env.DB_PORT,
    database: process.env.DB_NAME,
    password: process.env.DB_PASSWORD,
    ssl: {
        rejectUnauthorized: false,
        ca: fs.readFileSync('./pg-cert.crt').toString(),
    },
})

const options = {
    maxCandidates: 5,
    language: "en"
};

export const handler = async (event) => {
    await client.connect();
    const url = event.url;

    const vehicleNo = await getVehicleNo(url);
    console.log({ vehicleNo });

    const formattedVehicleNo = vehicleNo.substring(3).replace('-', '');
    console.log("formattedVehicleNo:", formattedVehicleNo);

    const query = `
        SELECT * FROM vehicles 
        WHERE REPLACE(REPLACE(vehicle_no, '-', ''), ' ', '') = REPLACE(REPLACE($1, '-', ''), ' ', '');
    `;

    try {
        const res = await client.query(query, [formattedVehicleNo]);
        if (res.rows.length > 0) {
            console.log('Vehicle Found:', res.rows[0]);
            return res.rows[0];
        } else {
            console.log('Vehicle Not Found');
            return null;
        }
    } catch (err) {
        console.error('Error checking vehicle:', err);
    } finally {
        await client.end();
    }

};


