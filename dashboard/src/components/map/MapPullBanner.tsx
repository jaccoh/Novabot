import { useCallback, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Loader2, MapPinOff } from 'lucide-react';
import { fetchAutoPullState, fetchMaps, type AutoPullState } from '../../api/client';
import { getSocket } from '../../api/socket';

/**
 * A fresh install without a cloud import has no map for the mower while the
 * mower still holds it; the server then asks the mower for it by itself
 * (server/src/services/mapPull.ts). This shows that, and asks for a backup
 * when nothing came back. Hidden as soon as there is a map.
 */
export function MapPullBanner({ sn }: { sn: string }) {
  const { t } = useTranslation();
  const [state, setState] = useState<AutoPullState>(null);

  const check = useCallback(() => {
    Promise.all([fetchMaps(sn), fetchAutoPullState(sn)])
      .then(([maps, pull]) => setState(maps.maps.length === 0 ? pull : null))
      .catch(() => { /* keep current state */ });
  }, [sn]);

  useEffect(() => {
    setState(null);
    check();
    const socket = getSocket();
    const changed = (event: { sn: string }) => { if (event.sn === sn) check(); };
    socket.on('maps:changed', changed);
    return () => { socket.off('maps:changed', changed); };
  }, [sn, check]);

  if (state === 'waiting' || state === 'pulling') {
    return (
      <div className="flex items-start gap-3 rounded-xl border border-emerald-500/30 bg-zinc-900/70 backdrop-blur-md px-4 py-3">
        <Loader2 className="w-4 h-4 mt-0.5 shrink-0 animate-spin text-emerald-400" />
        <div>
          <div className="text-sm font-semibold text-zinc-100">{t('mapPull.waitingTitle')}</div>
          <div className="text-xs text-zinc-400 mt-0.5">{t('mapPull.waitingBody')}</div>
        </div>
      </div>
    );
  }
  if (state !== 'empty') return null;
  return (
    <div className="flex flex-wrap items-center gap-3 rounded-xl border border-amber-500/40 bg-zinc-900/70 backdrop-blur-md px-4 py-3">
      <MapPinOff className="w-4 h-4 shrink-0 text-amber-400" />
      <div className="flex-1 min-w-[14rem]">
        <div className="text-sm font-semibold text-zinc-100">{t('mapPull.emptyTitle')}</div>
        <div className="text-xs text-zinc-400 mt-0.5">{t('mapPull.emptyBody')}</div>
      </div>
      <a
        href="/admin"
        className="inline-flex items-center rounded-lg border border-zinc-600 px-3 py-1.5 text-sm text-zinc-100 transition-colors hover:bg-zinc-800"
      >
        {t('mapPull.openAdmin')}
      </a>
    </div>
  );
}
