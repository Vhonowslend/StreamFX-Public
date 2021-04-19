const PATH = require("path");
const FS = require("fs/promises");
const CSV = require("csv");

(async () => {
	const parser = CSV.parse(
		await FS.readFile(process.argv[2]),
		{
			columns: true
		}
	);

	let unsorted_users = {};
	for await(const record of parser) {
		unsorted_users[record['Name']] = `https://www.patreon.com/user/creators?u=${record['User ID']}`;
	}
	let users = {};
	let keys = Object.keys(unsorted_users);
	for (let key of keys.sort()) {
		users[key] = unsorted_users[key];
	}

	FS.writeFile(
		PATH.join(__dirname, "patch-supporters-patreon.json"), 
		JSON.stringify(users, undefined, '\t'), 
		{
			encoding: 'utf8'
		}
	);
})();
