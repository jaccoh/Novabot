/**
 * Express 4 ignores a rejected promise from an async handler: the request
 * hangs and the rejection reaches the process, which Node 20 then exits.
 * Thirty-five handlers here are async without try/catch. As express-async-errors
 * does, wrap every non-error handler so a rejection goes to next(err) and
 * Express answers 500 instead.
 */
// @ts-expect-error express has no types for its internals
import Layer from 'express/lib/router/layer.js';

type Handler = (...args: unknown[]) => unknown;
const proto = Layer.prototype as { handle?: Handler; __handle?: Handler };

Object.defineProperty(proto, 'handle', {
  configurable: true,
  enumerable: true,
  get(this: typeof proto) { return this.__handle; },
  set(this: typeof proto, fn: Handler) {
    if (typeof fn === 'function' && fn.length < 4) {          // (req, res, next); error handlers keep 4 args
      const wrapped = function (this: unknown, req: unknown, res: unknown, next: (e?: unknown) => void) {
        const out = fn.call(this, req, res, next);
        if (out && typeof (out as Promise<unknown>).catch === 'function') (out as Promise<unknown>).catch(next);
        return out;
      };
      this.__handle = wrapped as unknown as Handler;
    } else {
      this.__handle = fn;
    }
  },
});
