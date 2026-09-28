import { useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { startSourceDockCycle, controlSourceDockCycle, type SourceDockCycle, type ZoneCopyAlignment } from '../../api/client';

interface Props {
  sn: string; source: string; canonical: string; disabled: boolean;
  onBusy: (busy: boolean) => void;
  onComplete: (alignment: ZoneCopyAlignment) => void;
}

export function SourceDockMeasurement({ sn, source, canonical, disabled, onBusy, onComplete }: Props) {
  const { t } = useTranslation();
  const [confirmed, setConfirmed] = useState(false);
  const [cycle, setCycle] = useState<SourceDockCycle | null>(null);
  const [error, setError] = useState<string | null>(null);
  const active = useRef<string | null>(null);
  const stopping = useRef(false);
  const mounted = useRef(true);
  const callbacks = useRef({ onBusy, onComplete });
  callbacks.current = { onBusy, onComplete };
  const cycleId = cycle?.cycleId;

  useEffect(() => {
    mounted.current = true;
    return () => {
      mounted.current = false;
      if (active.current) void controlSourceDockCycle(sn, source, active.current, 'stop').catch(() => {});
      active.current = null;
    };
  }, [sn, source]);

  useEffect(() => {
    if (!cycleId) return;
    let cancelled = false, inFlight = false;
    const tick = async () => {
      if (inFlight || active.current !== cycleId) return;
      inFlight = true;
      try {
        const next = await controlSourceDockCycle(sn, source, cycleId, stopping.current ? 'stop' : 'pulse');
        if (cancelled) return;
        setCycle(next);
        if (next.phase === 'done' || next.phase === 'error') {
          active.current = null;
          callbacks.current.onBusy(false);
          if (next.phase === 'done' && next.alignment && !stopping.current) callbacks.current.onComplete(next.alignment);
          else setError(next.error ?? t('map.copyAuto.stopped'));
        }
      } catch (err) {
        if (cancelled) return;
        stopping.current = true;
        active.current = null;
        void controlSourceDockCycle(sn, source, cycleId, 'stop').catch(() => {});
        setError(err instanceof Error ? err.message : String(err));
        setCycle(null);
        callbacks.current.onBusy(false);
      } finally { inFlight = false; }
    };
    void tick();
    const timer = setInterval(() => void tick(), 1_000);
    return () => { cancelled = true; clearInterval(timer); };
  }, [cycleId, sn, source, t]);

  const start = async () => {
    if (active.current || disabled || !confirmed) return;
    // randomUUID is unavailable on ordinary LAN HTTP origins.
    const bytes = crypto.getRandomValues(new Uint8Array(16));
    const hex = [...bytes].map(n => n.toString(16).padStart(2, '0')).join('');
    const id = `${hex.slice(0, 8)}-${hex.slice(8, 12)}-${hex.slice(12, 16)}-${hex.slice(16, 20)}-${hex.slice(20)}`;
    active.current = id; stopping.current = false;
    onBusy(true); setError(null); setCycle(null);
    try {
      const next = await startSourceDockCycle(sn, source, canonical, id);
      if (mounted.current && active.current === id) setCycle(next);
      else void controlSourceDockCycle(sn, source, id, 'stop').catch(() => {});
    } catch (err) {
      void controlSourceDockCycle(sn, source, id, 'stop').catch(() => {});
      if (mounted.current) { active.current = null; setError(err instanceof Error ? err.message : String(err)); onBusy(false); }
    }
  };
  const stop = () => {
    stopping.current = true;
    if (active.current) void controlSourceDockCycle(sn, source, active.current, 'stop').catch(() => {});
    setError(t('map.copyAuto.stopping'));
  };
  const running = !!active.current;
  return <div className="space-y-2 border-b border-gray-600 pb-2 mb-2">
    <p className="text-xs font-medium text-white">{t('map.copyAuto.title')}</p>
    <p className="text-[11px] text-gray-300">{t('map.copyAuto.description')}</p>
    {!running && <label className="flex items-start gap-2 text-[11px] text-gray-300">
      <input type="checkbox" className="mt-0.5" checked={confirmed} disabled={disabled} onChange={e => setConfirmed(e.target.checked)} />
      {t('map.copyAuto.confirm')}
    </label>}
    {running ? <>
      <p role="status" className="text-xs text-amber-300">{t(`map.copyAuto.phases.${cycle?.phase ?? 'starting'}`)}</p>
      <button type="button" onClick={stop} className="w-full rounded bg-red-700 px-3 py-2 text-sm font-semibold text-white">{t('map.copyAuto.stop')}</button>
    </> : <button type="button" onClick={() => void start()} disabled={disabled || !confirmed} className="w-full rounded bg-blue-700 px-2 py-1.5 text-xs text-white disabled:opacity-40">{t('map.copyAuto.start')}</button>}
    {error && <p role="alert" className="text-xs text-amber-300">{error}</p>}
  </div>;
}
