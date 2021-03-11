#!/bin/bash
cd spa
npm run build
cd ..
rm -rf pi-dist/www
cp -r dist/www pi-dist