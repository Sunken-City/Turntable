const webpack = require('webpack');
const GitRevisionPlugin = require('git-revision-webpack-plugin')
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require('copy-webpack-plugin');
const path = require('path');

const gitRevisionPlugin = new GitRevisionPlugin()

module.exports = {
    context: path.resolve(__dirname, 'src'),
    entry: "./index.tsx",
    module: {
        rules: [
            { test: /\.tsx?$/, use: { loader: 'ts-loader' } },
            { enforce: "pre", test: /\.js$/, loader: "source-map-loader" },
            {
                test: /\.(png|jpg|gif|styl|fbx|glb|gltf|svg|mp3|ogg)$/,
                use: [
                    {
                        loader: 'url-loader',
                        options: {
                            limit: 8192
                        }
                    },
                ]
            },
        ]
    },
    resolve: {
        extensions: ['.tsx', '.ts', '.js'],
    },
    output: {
        filename: 'main.[contenthash].js',
        publicPath: '/',
    },

    devServer: {
        historyApiFallback: true,
    },

    plugins: [
        new HtmlWebpackPlugin({
            title: "Turntable",
            filename: "index.html",
            template: path.join(__dirname, "src/index.html"),
            minify: true,
            hash: true,
            meta: {
                "apple-mobile-web-app-capable": "yes",
                "apple-mobile-web-app-status-bar-style": "black-translucent",
                "viewport": "width=device-width, initial-scale=1, maximum-scale=1, viewport-fit=cover",
                // disable this on the main game page because robots are harassing my websockets
                "robots": "noindex, nofollow",
            }
        }),
        new HtmlWebpackPlugin({
            filename: "404.html",
            title: "Menuapp",
            template: path.join(__dirname, "src/404.html"),
            chunks: []
        }),
        new webpack.DefinePlugin({
            'COMMITHASH': JSON.stringify(gitRevisionPlugin.commithash().substring(0, 6)),
        }),
        new CopyPlugin([
            { from: "images/loader_images", to: "images/loader_images" }
        ])
    ]
};