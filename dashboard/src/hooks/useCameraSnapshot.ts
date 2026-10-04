import { useCallback, useEffect, useRef, useState } from 'react';

/** Snapshot poll cadence (ms). ~2 fps, the same as the app's camera views. */
const CAMERA_POLL_MS = 500;
/** Consecutive snapshot failures before we surface the "unavailable" screen. */
const FAIL_THRESHOLD = 4;

/**
 * Live mower camera as a stream of short JPEG snapshots from the dashboard's
 * camera proxy, polled only while `enabled`.
 *
 * Poll short-lived snapshots instead of holding one permanent MJPEG stream
 * open in an <img>. A long-lived stream connection fills the browser's
 * per-host connection cap, and every retry / re-mount leaves a half-open
 * connection behind, so a view wedges on "unavailable" after a few opens (the
 * camera bug reported alongside #93). Short snapshot requests free the socket
 * immediately and self-heal after a transient hiccup.
 */
export function useCameraSnapshot(sn: string, topic: string, enabled: boolean) {
  const [error, setError] = useState(false);
  const [loading, setLoading] = useState(true);
  const [imageSrc, setImageSrc] = useState<string | null>(null);
  const activeRef = useRef(true);
  const blobUrlRef = useRef<string | null>(null);
  const failRef = useRef(0);

  const snapshotUrl =
    `/api/dashboard/camera/${encodeURIComponent(sn)}/snapshot` +
    `?topic=${encodeURIComponent(topic)}`;

  const fetchSnapshot = useCallback(async () => {
    if (!activeRef.current) return;
    try {
      const res = await fetch(`${snapshotUrl}&_t=${Date.now()}`);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const blob = await res.blob();
      // Een 200 met lege of niet-JPEG-body (camera bezet door een andere
      // kijker, stream nog niet klaar) toonde een kapot <img>-icoon; dat is
      // gewoon een gemiste frame → error-pad met retry.
      if (blob.size < 100 || (blob.type && !blob.type.startsWith('image/'))) {
        throw new Error('lege/ongeldige snapshot');
      }
      if (!activeRef.current) return;
      const oud = blobUrlRef.current;
      const url = URL.createObjectURL(blob);
      blobUrlRef.current = url;
      setImageSrc(url);
      // oude blob pas ná de render vrijgeven — direct revoken liet het
      // nog-getoonde plaatje een frame lang op een dode URL wijzen
      if (oud) setTimeout(() => URL.revokeObjectURL(oud), 1000);
      setLoading(false);
      setError(false);
      failRef.current = 0;
    } catch {
      if (!activeRef.current) return;
      // Tolerate the odd dropped frame; only show the error screen after a few
      // consecutive failures so a single hiccup does not wedge the view.
      failRef.current += 1;
      setLoading(false);
      if (failRef.current >= FAIL_THRESHOLD) setError(true);
    }
  }, [snapshotUrl]);

  useEffect(() => {
    if (!enabled) return;
    activeRef.current = true;
    failRef.current = 0;
    setLoading(true);
    setError(false);
    setImageSrc(null);
    fetchSnapshot();
    const interval = setInterval(fetchSnapshot, CAMERA_POLL_MS);
    return () => {
      activeRef.current = false;
      clearInterval(interval);
      if (blobUrlRef.current) {
        URL.revokeObjectURL(blobUrlRef.current);
        blobUrlRef.current = null;
      }
    };
  }, [enabled, fetchSnapshot]);

  const retry = useCallback(() => {
    failRef.current = 0;
    setError(false);
    setLoading(true);
    fetchSnapshot();
  }, [fetchSnapshot]);

  return { imageSrc, loading, error, retry };
}
