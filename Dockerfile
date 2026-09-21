FROM node:22-bookworm AS frontend
WORKDIR /app
ARG VITE_API_TOKEN=
ENV VITE_API_TOKEN=${VITE_API_TOKEN}
COPY package*.json ./
RUN npm ci
COPY . .
RUN npm run build

FROM python:3.12-slim
ENV PYTHONUNBUFFERED=1 KSPR_DATA_DIR=/data KSPR_ENVIRONMENT=production
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends git && rm -rf /var/lib/apt/lists/*
COPY pyproject.toml ./
COPY backend ./backend
COPY kspr_runtime.py ./
COPY skills ./skills
RUN pip install --no-cache-dir .
COPY --from=frontend /app/dist ./dist
RUN useradd --create-home --uid 10001 kspr && mkdir -p /data && chown -R kspr:kspr /app /data
USER kspr
EXPOSE 8000
VOLUME ["/data"]
HEALTHCHECK --interval=30s --timeout=5s CMD python -c "import urllib.request; urllib.request.urlopen('http://127.0.0.1:8000/api/v1/health')"
CMD ["uvicorn", "kspr_runtime:app", "--host", "0.0.0.0", "--port", "8000"]
