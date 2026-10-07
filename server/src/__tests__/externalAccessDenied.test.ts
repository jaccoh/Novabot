import { describe, it, expect, beforeEach } from 'vitest';
import bcrypt from 'bcrypt';
import { userRepo } from '../db/repositories/index.js';
import { externalAccessDenied } from '../middleware/externalAuthGate.js';

describe('externalAccessDenied (external gate roles)', () => {
  const hash = bcrypt.hashSync('secret-pw', 4);
  beforeEach(() => {
    userRepo.create('u-plain', 'someone@example.com', hash, 'someone');
    userRepo.create('u-admin-local', 'admin@local', bcrypt.hashSync('admin', 4), 'admin');
    userRepo.setRole('u-admin-local', 'is_admin', true);
    userRepo.setRole('u-admin-local', 'dashboard_access', true);
  });

  it('refuses an unknown or freshly registered account', () => {
    expect(externalAccessDenied(undefined)).toMatch(/unknown/);
    expect(externalAccessDenied('nope')).toMatch(/unknown/);
    expect(externalAccessDenied('u-plain')).toMatch(/no dashboard access/);
  });

  it('allows an account once it has dashboard access', () => {
    userRepo.setRole('u-plain', 'dashboard_access', true);
    expect(externalAccessDenied('u-plain')).toBeNull();
  });

  it('refuses admin@local while it still has the default password, allows it after a change', () => {
    expect(externalAccessDenied('u-admin-local')).toMatch(/default password/);
    userRepo.updatePassword('u-admin-local', bcrypt.hashSync('something-else', 4));
    expect(externalAccessDenied('u-admin-local')).toBeNull();
  });
});
