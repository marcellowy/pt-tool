#!/bin/sh

DATE=$(date +"%Y%m%d%H%M%S")
mv /app/pt-tool/pt-tool /app/pt-tool/pt-tool.$DATE
mv build/pt-tool /app/pt-tool/pt-tool
supervisorctl restart pt-tool