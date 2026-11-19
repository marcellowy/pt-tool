#!/bin/sh

upx build/pt-tool

rm -f /app/pt-tool/pt-tool.old
mv /app/pt-tool/pt-tool /app/pt-tool/pt-tool.old
mv build/pt-tool /app/pt-tool/pt-tool
supervisorctl restart pt-tool