import { describe, it, expect } from 'vitest';
import express from 'express';
import '../middleware/asyncErrors.js';

async function respond(app: express.Express, path: string): Promise<number> {
  const server = app.listen(0);
  try {
    const { port } = server.address() as { port: number };
    const r = await fetch(`http://127.0.0.1:${port}${path}`);
    return r.status;
  } finally {
    server.close();
  }
}

describe('asyncErrors: a rejected async handler answers 500 instead of hanging', () => {
  it('turns a rejection into next(err)', async () => {
    const app = express();
    app.get('/boom', async () => { throw new Error('nope'); });
    app.get('/ok', async (_req, res) => { res.json({ ok: true }); });
    app.use((_err: unknown, _req: express.Request, res: express.Response, _next: express.NextFunction) => { res.status(500).json({ caught: true }); });
    expect(await respond(app, '/boom')).toBe(500);
    expect(await respond(app, '/ok')).toBe(200);
  });
});
