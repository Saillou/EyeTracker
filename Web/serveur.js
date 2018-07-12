const http 			= require('http');
const express 	= require('express');
const fs				= require('fs');

const PORT_USED = 3001;

const app = express();
const server = http.Server(app);

// Start app
app.use('/Style', express.static(__dirname + '/views/Style'));

app.listen(PORT_USED, function () {
  console.log('Server ready.')
})

// Path
app.get('/', function (req, res) {
	var files ={'Movies' : fs.readdirSync('./Recordings/')};
	res.render('index.ejs', files);
})

app.get('/download', function (req, res) {
	var file = __dirname + '/Recordings/' + req.query.id;
	res.download(file);
})
