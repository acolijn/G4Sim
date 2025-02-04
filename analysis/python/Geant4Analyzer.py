import uproot
import numpy as np
import awkward as ak
from matplotlib.patches import Circle, Rectangle
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from XAMSPlotter import XAMSPlotter

from RunManager import RunManager
from mendeleev import element

def is_jagged(array):
    """Check if the given array is a jagged array."""
    return isinstance(array, ak.highlevel.Array) and isinstance(array.layout, ak.contents.ListOffsetArray)


class Geant4Analyzer:
    def __init__(self, run_id, label="", first_only=False, start_index=None, file_count=None):
        """
        Initializes the analyzer with the given file path.

        Args:
            file_path (str): The path to the ROOT file.
            label (str, optional): The label to use for the plot.
        """

        manager = RunManager("../../run/rundb.json")

        self.file_paths = manager.get_output_root_files(run_id, first_only=first_only, start_index=start_index, file_count=file_count)
        self.settings = manager.get_run_settings(run_id, convert_units=True)
        self.geometry = manager.get_geometry(run_id)   
        self.label = ""

        if label == "":
            if 'ion' in self.settings['gps_settings']:
                txt = self.settings['gps_settings']['ion']
                Z = txt.split(" ")[0]
                A = txt.split(" ")[1]
                element_symbol = element(int(Z)).symbol
                label_text = 'ion: $^{{{:2d}}}${:s}'.format(int(A), element_symbol)
                self.label = label_text
            else:
                self.label = ""
        else:
            self.label = label

        self.raw = None
        self.data = {}

        print(f"Initialized Geant4Analyzer with run_id={run_id}, label={self.label}")	
        self.load_data()

    def load_data(self):
        """
        Loads the raw data from the file or list of files.

        Args:
            file_paths (str or list): Path or list of paths to the ROOT files.

        Raises:
            FileNotFoundError: If any file is not found.
        """

        print(f"Loading data from {self.file_paths}")
        if isinstance(self.file_paths, str):
            self.file_paths = [self.file_paths]

        data_list = []

        for file_path in self.file_paths:
            try:
                print(f"Loading {file_path}")
                root = uproot.open(file_path)
                data = root["ev"].arrays(library="ak")  # Use awkward array
                data_list.append(data)
            except FileNotFoundError:
                raise FileNotFoundError(f"File not found: {file_path}")

        if data_list:
            # Concatenate awkward arrays
            self.raw = ak.concatenate(data_list, axis=0)

            # Add derived variables
            self.raw['r'] = np.sqrt(self.raw['xh']**2 + self.raw['yh']**2)

            print(f"Data loaded from {len(self.file_paths)} files")
        else:
            print("No data loaded")

        print(f"Data loaded from {self.file_paths}")


    def load_data_obsolete(self):
        """
        Loads the raw data from the file.

        Raises:
            FileNotFoundError: If the file is not found.
        """
        if isinstance(self.file_paths, str):
            self.file_paths = [self.file_paths]

        for file in self.file_paths:

            root = uproot.open(file)
            self.raw = root["ev"].arrays()
            # add derived variables
            # radius
            self.raw['r'] = np.sqrt(self.raw['xh']**2 + self.raw['yh']**2)

        print(f"Data loaded from {self.file_paths}")

    def preprocess_data(self, cut=None, cut_hit=None):
        """
        Preprocesses the raw data by applying filters and converting it to a format suitable for analysis.

        Args:
            cut (array-like, optional): The primary cut to apply to the data.
            cut_hit (array-like, optional): Additional cut for jagged arrays.
        
        Raises:
            ValueError: If the data is not loaded. Call load_data() first.
        """
        if self.raw is None:
            raise ValueError("Data not loaded. Call load_data() first.")
        
        if cut is None:
            cut = cut
        else:
            cut = cut(self.raw)
            
        if cut_hit is None:
            cut_hit = cut
        else:
            cut_hit = cut_hit(self.raw) & cut

        for field in self.raw.fields:
            data_field = self.raw[field]
            if is_jagged(data_field):
                # Flatten jagged arrays
                
                # make sure that you do not apply the cuts on the hits on the other fields   
                if ( (field == 'edet') or (field == 'ndet') or (field == 'ncomp') or (field == 'nphot') ):
                    data_field = data_field[cut]
                else:
                    data_field = ak.flatten(data_field[cut_hit])
            else:
                data_field = data_field[cut]

            self.data[field] = ak.to_numpy(data_field)


    def plot_histogram(self, variable, ax=None, bins=50, range=None, label=None, show=True, errorbar=False):
        """
        Plots a histogram of the given variable or points with error bars.

        Args:
            variable (str): The variable to plot.
            ax (matplotlib.axes.Axes, optional): The axis to plot on. If None, a new figure is created.
            bins (int or array-like, optional): The number of bins or bin edges.
            range (tuple, optional): The range of the histogram.
            label (str, optional): Label for the plot.
            show (bool, optional): Whether to display the plot.
            errorbar (bool, optional): Whether to plot points with error bars instead of a histogram.

        Returns:
            matplotlib.axes.Axes: The axis object.

        Raises:
            ValueError: If the variable is not found in the preprocessed data.
        """ 
        if variable not in self.data:
            raise ValueError(f"Variable '{variable}' not found in preprocessed data.")

        if ax is None:
            fig, ax = plt.subplots()

        # use the event weights for the event variables, otherwise use the hit weights
        weights = self.data['w'] if len(self.data['w']) == len(self.data[variable]) else self.data['wh']

        # Calculate histogram and bin properties
        hist, bin_edges = np.histogram(self.data[variable], weights=np.exp(weights), bins=bins, range=range)
        bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])  # Calculate bin centers
        errors = np.sqrt(hist)  # Poisson error for each bin

        print(f"integral {label}=", np.sum(hist))

        if errorbar:
            # Plot points with error bars using caret markers and reduced marker size
            ax.errorbar(bin_centers, hist, yerr=errors, fmt='^', markersize=3, label=label)
        else:
            # Plot histogram
            ax.hist(self.data[variable], weights=np.exp(weights), bins=bins, range=range, histtype='step', label=label)

        # Set labels based on variable
        if variable == 'r':
            ax.set_xlabel('radius (mm)')
        elif (variable == 'xp') or (variable == 'xh'):
            ax.set_xlabel('x (mm)')
        elif (variable == 'yp') or (variable == 'yh'):
            ax.set_xlabel('y (mm)')
        elif (variable == 'zp') or (variable == 'zh'):
            ax.set_xlabel('z (mm)')
        elif (variable == 'eh') or (variable == 'e'):
            ax.set_xlabel('Energy (keV)')
        else:
            ax.set_xlabel(variable)
    
        ax.set_ylabel('Counts')

        if show:
            ax.legend(frameon=False)

        return ax
    
    def plot_2d_histogram_with_detector(self, view="xy", bins=500, range=None, ax=None, saveFigFilename=None):
        """
        Plots a 2D histogram of the hits and overlays the detector geometry if available.

        Args:
            view (str): The view for the plot, either "xy" or "rz". Default is "xy".
            bins (int or array-like): The number of bins for the histogram. Default is 500.
            range (tuple or list): The range for the histogram. Default is set based on view.
            ax (matplotlib.axes.Axes): The axis to plot on. If None, a new figure is created.
        """
        if ax is None:
            fig, ax = plt.subplots()
            fig.set_size_inches(5, 5)

        # Set default ranges based on view
        if range is None:
            if view == "xy":
                range = [[-150, 150], [-35, 265]]  # Default range for x-y
            elif view == "rz":
                range = [[0, 300], [-125, 175]]  # Default range for r-z

        # Plot 2D histogram
        if view == "xy":
            h = ax.hist2d(self.data['xh'], self.data['yh'], bins=bins, range=range, cmap='viridis', norm=LogNorm())


        elif view == "rz":
            h = ax.hist2d(self.data['r'], self.data['zh'], bins=bins, range=range, cmap='viridis', norm=LogNorm())

        # show source position
        self.show_source_position(ax, view)

        # Plot detector geometry if the detector type is 'xams'
        if "xams" in self.geometry.get('detector', "").lower():
            gpl = XAMSPlotter(self.geometry)
            gpl.plot_geometry(ax=ax, view=view)

        # Label axes
        if view == "xy":
            ax.set_xlabel("$x$ (mm)")
            ax.set_ylabel("$y$ (mm)")
        elif view == "rz":
            ax.set_xlabel("$r$ (mm)")
            ax.set_ylabel("$z$ (mm)")

        # Show plot and optionally save to a file
        if saveFigFilename is not None:
            plt.savefig(saveFigFilename)

        #plt.show()

    def show_source_position(self, ax, view):
        """
        Plots the source position on the given axis based on the specified view.

        Parameters:
        ax (matplotlib.axes.Axes): The matplotlib axes object where the source position will be plotted.
        view (str): The view type for plotting. It can be either "xy" or "rz".

        Notes:
        - The function expects 'gps_settings' and 'posCentre' to be present in the 'settings' attribute of the class.
        - The source position is scaled by a factor of 10.
        - For the "xy" view, the source position is plotted in the XY plane.
        - For the "rz" view, the source position is plotted in the RZ plane, where R is the radial distance from the origin.

        Raises:
        KeyError: If 'gps_settings' or 'posCentre' is not found in the settings.
        """
        if 'gps_settings' in self.settings:
            if 'posCentre' in self.settings['gps_settings']:
                pos = self.settings['gps_settings']['posCentre']
                pos = pos.replace(" cm", "").split()
                pos = [float(p) for p in pos]


                if view == "xy":
                    print("source position: ", pos)
                    print("source position: ", pos[0], pos[1])
                    x_source = float(pos[0])*10.
                    y_source = float(pos[1])*10.
                    ax.plot([x_source, x_source], [0., y_source+100], '--', color='blue', linewidth=0.5)
                    ax.plot(x_source, y_source, 'bx', markersize=3)
                elif view == "rz":
                    r_source = np.sqrt(float(pos[0])*10.+float(pos[1])**2)*10.
                    z_source = float(pos[2])*10.
                    ax.plot([0, 400], [z_source, z_source], '--', color='blue', linewidth=0.5)
                    ax.plot(r_source, z_source, 'bx', markersize=3)

    def analyze_event_classifications(self, cut=None, cut_hit=None, ax=None, show=True):
        """
        Analyzes and prints the combinations of classifications in the current cut.
        It processes each event and categorizes it based on the event type bits set.

        Args:
            cut (callable, optional): Primary cut for filtering events.
            cut_hit (callable, optional): Additional cut for filtering events based on hit-level cuts.
            ax (matplotlib.axes.Axes, optional): The axis to plot on. If None, a new figure is created.
            show (bool, optional): Whether to display the plot.

        Returns:
            matplotlib.axes.Axes: The axis object.
        """

        if self.raw is None:
            raise ValueError("Data not loaded. Call load_data() first.")

        # Apply both primary cut and hit-level cut if provided
        if cut is not None:
            primary_cut_mask = cut(self.raw)
        else:
            primary_cut_mask = ak.ones_like(self.raw['type'], dtype=bool)

        if cut_hit is not None:
            hit_cut_mask = cut_hit(self.raw)
            combined_cut_mask = primary_cut_mask & ak.any(hit_cut_mask, axis=1)
        else:
            combined_cut_mask = primary_cut_mask

        # Filter event types using the combined mask
        filtered_event_types = self.raw['type'][combined_cut_mask]

        #TEMP 
        filtered_event_ID = self.raw['ev'][combined_cut_mask]
        
        total_events = len(filtered_event_types)
        print(f"Total number of events: {total_events}")

        # Initialize counts for all possible combinations of event types
        classification_counts = {}

        # Process each event type combination
        for event_type in filtered_event_types:
            event_type_int = int(event_type)
            
            # Create a unique string representation of the combination of bits set
            classification = []
            if event_type_int & 1:
                classification.append('Dir')
            elif event_type_int & 2:
                classification.append('Scat')
            if event_type_int & 4:
                classification.append('Brem')
            if event_type_int & 8:
                classification.append('Brem_esc')
            if event_type_int & 16:
                classification.append('Esc')
            
            classification_str = ' + '.join(classification) if classification else 'NONE'
            
            # Increment the count for this classification combination
            if classification_str not in classification_counts:
                classification_counts[classification_str] = 0
            classification_counts[classification_str] += 1

        # Now, plot the counts
        if ax is None:
            fig, ax = plt.subplots(figsize=(10, 8))

        classifications = list(classification_counts.keys())
        counts = [classification_counts[cls] for cls in classifications]

        bars = ax.bar(classifications, counts, color='skyblue')

        # Add count labels above each bar
        for bar, count in zip(bars, counts):
            yval = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2.0, yval + 0.5, int(count), ha='center', va='bottom')

        ax.set_xlabel('Event Classification Combinations')
        ax.set_ylabel('Number of Events')
        ax.set_title('Number of Events per Classification Combination')
        ax.tick_params(axis='x', rotation=90)
        ax.set_ylim(0, max(counts) * 1.1)
        plt.tight_layout()

        if show:
            plt.show()

        return ax

    def print_event_data(self, event_id):
        """
        Print data for a specific event ID from raw data.
        
        Args:
            event_id (float): The event ID to look up.
        """
        if self.raw is None:
            raise ValueError("Data not loaded. Call load_data() first.")

        # Locate the specific event by event ID
        event_data = self.raw[self.raw["ev"] == event_id]

        if len(event_data) == 0:
            print(f"Event {event_id} not found.")
            return

        # Print all fields for the specified event
        print(f"Data for Event ID {event_id}:")
        for field in event_data.fields:
            print(f"{field}: {event_data[field]}")


