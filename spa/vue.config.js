module.exports = {
  pages: {
    'index': {
      entry: './src/main.js',
      template: 'public/index.html',
      chunks: [ 'chunk-vendors', 'chunk-common', 'index' ]
    },
    'service': {
      entry: './src/service/main.js',
      template: 'public/service.html',
      title: 'service',
      chunks: [ 'chunk-vendors', 'chunk-common', 'service' ]
    }
  },
  productionSourceMap: false,
  outputDir : "../dist/www",
  publicPath : "",
  runtimeCompiler: true,
  pluginOptions: {
    i18n: {
      locale: 'en',
      fallbackLocale: 'en',
      localeDir: 'locales',
      enableInSFC: true
    }
  }
}
