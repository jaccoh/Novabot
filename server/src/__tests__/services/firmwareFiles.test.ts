import { describe, it, expect } from 'vitest';
import path from 'path';
import { safeFirmwarePath, isFirmwareFileName, firmwareSourceAllowed } from '../../services/firmwareFiles.js';

const dir = '/srv/firmware';

describe('safeFirmwarePath', () => {
  it('accepts a bare name inside the directory', () => {
    expect(safeFirmwarePath(dir, 'mower_firmware_v6.0.2-custom-46.deb')).toBe(path.join(dir, 'mower_firmware_v6.0.2-custom-46.deb'));
  });
  it('rejects traversal, nested paths, dots and non-strings', () => {
    for (const bad of ['../../data/.jwt_secret', '..', '.', 'a/b.deb', '/etc/passwd', '', undefined, 42]) {
      expect(safeFirmwarePath(dir, bad)).toBeNull();
    }
  });
});

describe('isFirmwareFileName', () => {
  it('matches manifest names only', () => {
    expect(isFirmwareFileName('charger_firmware_v0.4.0.bin')).toBe(true);
    for (const bad of ['../x.deb', 'x.json', '.hidden.deb', 'a b.deb', '']) expect(isFirmwareFileName(bad)).toBe(false);
  });
});

describe('firmwareSourceAllowed', () => {
  it('allows only https on the manifest host', () => {
    expect(firmwareSourceAllowed('https://downloads.ramonvanbruggen.nl/mower_firmware_v6.0.2-custom-46.deb')).toBe(true);
    for (const bad of ['http://downloads.ramonvanbruggen.nl/x.deb', 'https://attacker.example/x.deb', 'https://192.168.0.1/x.deb', 'not a url', 7]) {
      expect(firmwareSourceAllowed(bad)).toBe(false);
    }
  });
});
