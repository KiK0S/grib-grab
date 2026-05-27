const SHROOMS_RUNTIME_CACHE = "shrooms-runtime-v1";
const SHROOMS_RUNTIME_ASSETS = [
  "shrooms.js",
  "shrooms.wasm",
];
const SHROOMS_RUNTIME_BASENAMES = new Set(SHROOMS_RUNTIME_ASSETS);
let runtimeRefresh = null;

function requestBasename(request) {
  const url = new URL(request.url);
  const parts = url.pathname.split("/");
  return parts[parts.length - 1] || "";
}

function runtimeAssetRequest(name) {
  return new Request(new URL(name, self.registration.scope).href, { cache: "reload" });
}

async function refreshRuntimeCache() {
  if (runtimeRefresh) return runtimeRefresh;
  runtimeRefresh = (async () => {
    const cache = await caches.open(SHROOMS_RUNTIME_CACHE);
    const requests = SHROOMS_RUNTIME_ASSETS.map(runtimeAssetRequest);
    const responses = await Promise.all(requests.map((request) => fetch(request)));
    if (!responses.every((response) => response && response.ok)) return;
    await Promise.all(
        responses.map((response, index) => cache.put(requests[index], response.clone())),
    );
  })()
      .catch(() => {})
      .finally(() => {
        runtimeRefresh = null;
      });
  return runtimeRefresh;
}

async function cacheFirst(request, refresh) {
  const cache = await caches.open(SHROOMS_RUNTIME_CACHE);
  const cached = await cache.match(request);
  if (cached) return cached;
  await refresh;
  const refreshed = await cache.match(request);
  if (refreshed) return refreshed;
  return fetch(request);
}

self.addEventListener("install", (event) => {
  event.waitUntil(refreshRuntimeCache().then(() => self.skipWaiting()));
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
      caches.keys().then((keys) =>
          Promise.all(
              keys
                  .filter((key) => key.startsWith("shrooms-runtime-") &&
                      key !== SHROOMS_RUNTIME_CACHE)
                  .map((key) => caches.delete(key)),
          )).then(() => self.clients.claim()),
  );
});

self.addEventListener("fetch", (event) => {
  if (event.request.method !== "GET") return;
  if (!SHROOMS_RUNTIME_BASENAMES.has(requestBasename(event.request))) return;
  const refresh = refreshRuntimeCache();
  event.waitUntil(refresh);
  event.respondWith(cacheFirst(event.request, refresh));
});
