# Copyright 2019 Brian T. Park
#
# MIT License

import logging
import os

class JavaGenerator:
    """Create a Java source file that contains the names of zones
    supported by zonedb and zonedbx. Will be used by the Java program
    to generate the validation data beyond the 2038 limit of pytz.
    """

    ZONES_FILE = """\
# This file was generated by the following script:
#
# $ {invocation}
#
# using the TZ Database files
#
#  {tz_files}
#
# from https://github.com/eggert/tz/releases/tag/{tz_version}
#
# DO NOT EDIT

# numZones: {numZones}
{zoneStrings}
"""

    def __init__(self, invocation, tz_version, tz_files, scope, zones_map):
        """
        Args:
            zones_map (dict): {full_name -> ZoneEra[]}
        """
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.zones_map = zones_map

        self.file_name = 'zones.txt'

    def generate_files(self, output_dir):
        """Generate a text file that contains the list of zones.
        """
        self._write_file(output_dir, self.file_name, self._generate_zones())

    def _generate_zones(self):
        zone_strings = ""
        for name, eras in sorted(self.zones_map.items()):
            zone_strings += name + '\n'
        return self.ZONES_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            tz_files=', '.join(self.tz_files),
            scope=self.scope,
            zoneStrings=zone_strings,
            numZones=len(self.zones_map))

    def _write_file(self, output_dir, filename, content):
        full_filename = os.path.join(output_dir, filename)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            print(content, end='', file=output_file)
        logging.info("Created %s", full_filename)
