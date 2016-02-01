import subprocess


class Tool(object):
    def __init__(self, verbose):
        self.verbose = verbose

    def run(self, args, verbose=None):
        if verbose is None:
            verbose = self.verbose
        p = subprocess.Popen(args, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        if verbose:
            print ' '.join(args)
        stdout, _ = p.communicate()
        if verbose:
            print stdout
        return p.returncode, stdout
