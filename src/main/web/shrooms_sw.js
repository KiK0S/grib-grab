const SHROOMS_RUNTIME_CACHE = "shrooms-runtime-v1";
const SHROOMS_RUNTIME_BASENAMES = new Set([
  "shrooms.js",
  "shrooms.wasm",
]);

function requestBasename(request) {
  const url = new URL(request.url);
  const parts = url.pathname.split("/");
  return parts[parts.length - 1] || "";
}

async function networkFirst(request) {
  const cache = await caches.open(SHROOMS_RUNTIME_CACHE);
  try {
    const response = await fetch(request);
    if (response && response.ok) {
      await cache.put(request, response.clone());
    }
    return response;
  } catch (err) {
    const cached = await cache.match(request);
    if (cached) return cached;
    throw err;
  }
}

self.addEventListener("install", (event) => {
  event.waitUntil(self.skipWaiting());
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
  event.respondWith(networkFirst(event.request));
});
