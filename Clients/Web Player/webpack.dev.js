const merge = require('webpack-merge');
const webpack = require('webpack');
const common = require('./webpack.common.js');
const path = require('path');

module.exports = merge(common, {
    mode: 'development',
    devtool: 'inline-source-map',

    devServer: {
        contentBase: path.join(__dirname, 'dist'),
        compress: true,
        port: 80,
        // don't use localhost or it's not visible on the local network
        host: "0.0.0.0",
    },

    plugins: [
        new webpack.DefinePlugin({
            'WEBPACK_MODE': JSON.stringify("development"),
        }),
    ]
});
