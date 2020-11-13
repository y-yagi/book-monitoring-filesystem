const { exec } = require('child_process');
const watchman = require('fb-watchman');
const client = new watchman.Client();

sub = {
  // Match any `.js` file in the dir_of_interest
  expression: ["allof", ["match", "*.re"]],
  // Which fields we're interested in
  fields: ["name", "size", "mtime_ms", "exists", "type"]
};

client.command(['watch-project', process.cwd()], function(error, resp) {
  if (error) {
    console.log('watch failed: ', error);
    return;
  }
  console.log('watch: ', process.cwd())
  if ('relative_path' in resp) {
    make_subscription(client, resp.watch, resp.relative_path)
  } else {
    make_subscription(client, resp.watch, null)
  }
});

function make_subscription(client, watch, relative_path) {
  sub = {
    expression: ["allof", ["match", "*.re"]],
    fields: ["name", "size", "mtime_ms", "exists", "type"]
  };
  if (relative_path) {
    sub.relative_root = relative_path;
  }

  client.command(['subscribe', watch, 're-file-subscription', sub],
    function (error, resp) {
      if (error) {
        // Probably an error in the subscription criteria
        console.error('failed to subscribe: ', error);
        return;
      }
      console.log('subscription ' + resp.subscribe + ' established');
    });

  // Subscription results are emitted via the subscription event.
  // Note that this emits for all subscriptions.  If you have
  // subscriptions with different `fields` you will need to check
  // the subscription name and handle the differing data accordingly.
  // `resp`  looks like this in practice:
  //
  // { root: '/private/tmp/foo',
  //   subscription: 'mysubscription',
  //   files: [ { name: 'node_modules/fb-watchman/index.js',
  //       size: 4768,
  //       exists: true,
  //       type: 'f' } ] }
  client.on('subscription', function (resp) {
    if (resp.subscription !== 're-file-subscription') return;

    resp.files.forEach(function (file) {
      exec('bundle exec rake pdf', (err, stdout, stderr) => {
        if (err) {
          // node couldn't execute the command
          return;
        }

        if (stdout !== '') console.log(`stdout: ${stdout}`);
        if (stderr !== '') console.log(`stderr: ${stderr}`);
      });
    });
  });
}
