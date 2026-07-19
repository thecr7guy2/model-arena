"use client";

const KEY = "spark_showdown_ratings_v1";

export function loadRatings() {
  if (typeof window === "undefined") return {};
  try { return JSON.parse(localStorage.getItem(KEY)) || {}; }
  catch { return {}; }
}

export function saveRating(taskId, modelId, score) {
  const r = loadRatings();
  (r[taskId] = r[taskId] || {})[modelId] = score;
  localStorage.setItem(KEY, JSON.stringify(r));
  window.dispatchEvent(new Event("ratings-changed"));
}

export function ratingOf(taskId, modelId) {
  return loadRatings()[taskId]?.[modelId];
}

export function clearRatings() {
  localStorage.removeItem(KEY);
  window.dispatchEvent(new Event("ratings-changed"));
}
