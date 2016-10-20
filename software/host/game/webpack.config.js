var path = require('path');
var webpack = require('webpack')

var ROOT_PATH = path.resolve(__dirname)
var CLIENT_PATH = path.resolve(ROOT_PATH, '.')
var BUILD_PATH = path.resolve(ROOT_PATH, 'static');

var TARGET = process.env.npm_lifecycle_event;
process.env.BABEL_ENV = TARGET;

module.exports = {
    target: 'node',
    context: __dirname,
    entry: [
        path.resolve(CLIENT_PATH, 'app/index.jsx')
    ],
    
    resolve: {
        extensions: [ '', '.js', '.jsx' ]
    },

    output: {
        path: BUILD_PATH,
        publicPath: '/client/',
        filename: 'bundle.js'
    },
    
    devtool: "eval-source-map",

    module: {
        loaders: [
            // CSS
            {
                test: /\.css$/,
                loaders: ['style', 'css'],
                include: CLIENT_PATH
            },

            // JS
            {
                test: /\.js$/,
                loader: 'babel',
                exclude: 'node_modules'
            },

            // JSX
            {
                test: /\.jsx$/,
                loaders: ['babel'],
                include: CLIENT_PATH
            },

            // Images
            {
                test: /\.(png|jpg)$/,
                loader: 'url-loader?limit=10'
            },
            
            // Images
            {
                test: /\.json$/,
                loader: 'json'
            }

  
        ]
    },
    
    plugins: [
        //new webpack.optimize.OccuranceOrderPlugin(),
        new webpack.NoErrorsPlugin()
    ]

};
