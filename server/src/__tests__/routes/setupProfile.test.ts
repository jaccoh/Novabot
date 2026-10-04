import { afterAll, afterEach, describe, expect, it, vi } from 'vitest';
import express from 'express';
import request from 'supertest';
import { mkdtempSync, writeFileSync } from 'fs';
import { tmpdir } from 'os';
import { join } from 'path';

vi.mock('../../services/lfiCloud.js', () => ({
  callLfiCloud: vi.fn(),
  encryptCloudPassword: vi.fn(() => 'encrypted-password'),
  makeLfiHeaders: vi.fn(() => ({})),
  LFI_CLOUD_HOST: 'app.lfibot.com',
  LFI_CLOUD_SERVERNAME: 'app.lfibot.com',
}));

vi.mock('../../services/cloudWorkRecordsImport.js', () => ({
  importCloudWorkRecords: vi.fn().mockResolvedValue({ inserted: 0, skipped: 0 }),
}));

vi.mock('../../services/portableBackup.js', () => ({
  createBundleFromDb: vi.fn().mockResolvedValue(null),
}));

import { setupRouter } from '../../routes/setup.js';

const app = express();
app.use('/api/setup', setupRouter);
const server = app.listen(0);
afterAll(() => new Promise<void>(r => { server.close(() => r()); }));

const FAKE_DER = Buffer.from('not-a-real-certificate');
const dir = mkdtempSync(join(tmpdir(), 'opennova-cert-'));
const certPath = join(dir, 'server.crt');
writeFileSync(
  certPath,
  `-----BEGIN CERTIFICATE-----\n${FAKE_DER.toString('base64')}\n-----END CERTIFICATE-----\n`,
);

afterEach(() => { delete process.env.CERT_PATH; });

describe('GET /api/setup/profile (iOS .mobileconfig)', () => {
  it('carries only the CA certificate, never a DNS payload', async () => {
    process.env.CERT_PATH = certPath;
    const res = await request(server).get('/api/setup/profile');
    expect(res.status).toBe(200);
    expect(res.headers['content-type']).toContain('application/x-apple-aspen-config');
    const body = res.text;
    expect(body).toContain('<string>com.apple.security.root</string>');
    expect(body).toContain(`<data>${FAKE_DER.toString('base64')}</data>`);
    // The payload that took over all DNS on iOS (Apple reserves it for
    // DNS-over-HTTPS/TLS). Must stay out.
    expect(body).not.toMatch(/dnsSettings/i);
    expect(body).not.toContain('ServerAddresses');
  });

  it('is a 404, not an empty profile, when the certificate is missing', async () => {
    process.env.CERT_PATH = join(dir, 'missing.crt');
    const res = await request(server).get('/api/setup/profile');
    expect(res.status).toBe(404);
  });
});
