#!/bin/bash

chmod +x pt-tool
rm -rf logs/pt-tool.log
supervisorctl restart pt-tool