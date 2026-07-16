import { createRequire } from 'node:module';
import { existsSync, readdirSync } from 'node:fs';
import { join, resolve } from 'node:path';

const employeeId = process.argv[2]?.trim() || 'EMP-001';
const pnpmStore = resolve('node_modules', '.pnpm');
const packageDirectory = readdirSync(pnpmStore).find((name) => name.startsWith('qrcode@'));
if (!packageDirectory) throw new Error('The installed qrcode package was not found. Run the mobile dependency install first.');

const modulePath = join(pnpmStore, packageDirectory, 'node_modules', 'qrcode');
if (!existsSync(modulePath)) throw new Error(`The qrcode module is incomplete at ${modulePath}.`);

const require = createRequire(import.meta.url);
const QRCode = require(modulePath);
const outputPath = resolve(`worker-${employeeId}-qr.png`);
const payload = JSON.stringify({ version: 1, employeeId });

await QRCode.toFile(outputPath, payload, {
  width: 600,
  margin: 4,
  color: { dark: '#101923', light: '#FFFFFF' },
});

console.log(`Worker QR created: ${outputPath}`);
console.log(`Payload: ${payload}`);
