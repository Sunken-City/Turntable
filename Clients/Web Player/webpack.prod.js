const webpack = require('webpack');
const merge = require('webpack-merge');
const common = require('./webpack.common.js');
var path = require('path');

let FaviconsWebpackPlugin = require('favicons-webpack-plugin');

module.exports = merge(common, {
    mode: 'production',
    // TODO: reenable using the plugin directly and upload them to Azure but to a private storage.
    // we don't want to make hacker's life even easier
    // devtool: 'source-map',

    module: {
        rules: [
            {
                test: /\.(jpg|png|gif|svg)$/,
                loader: 'image-webpack-loader',
                // Specify enforce: 'pre' to apply the loader
                // before url-loader/svg-url-loader
                // and not duplicate it in rules with them
                enforce: 'pre'
            }
        ],
    },

    plugins: [
        new FaviconsWebpackPlugin({
            // Your source logo
            logo: path.resolve(__dirname, './src/images/logo.png'),
            title: 'Turntable',
            favicons: {
                appName: 'Turntable',
                appShortName: 'Turntable',
                developerName: 'Sunken City',
                developerURL: "cloudyga.me",
                background: '#000000',
                theme_color: '#523a2a',
            }
        }),
        new webpack.DefinePlugin({
            'WEBPACK_MODE': JSON.stringify("production"),
        }),
    ]
});