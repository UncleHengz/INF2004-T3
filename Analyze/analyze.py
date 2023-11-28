from collections import Counter
import matplotlib.pyplot as plt

# Read the contents of the packets.txt file
with open('packet_log.txt', 'r') as file:
    data = file.read()

# Convert hex string to ASCII
data = data.replace(' ', '').replace('\n', '')  # Remove spaces and newlines

# Convert hex to ASCII characters
ascii_data = bytes.fromhex(data).decode('utf-8', errors='ignore')  # Convert hex to ASCII

# Insert a newline before "POST"
ascii_data = ascii_data.replace('POST', '\nPOST')

# Extract usernames and passwords using regex pattern
import re
matches = re.findall(r'username=([\w]+)&password=([\w]+)', ascii_data)

# Count occurrences of usernames and passwords
username_counter = Counter()
password_counter = Counter()
for username, password in matches:
    username_counter[username] += 1
    password_counter[password] += 1

# Get top 5 most common usernames and passwords
top_5_usernames = dict(username_counter.most_common(5))
top_5_passwords = dict(password_counter.most_common(5))

# Create bar charts for the top 5 most common usernames and passwords
def create_bar_chart(data, title):
    plt.bar(data.keys(), data.values())
    plt.title(title)
    plt.xlabel('Credentials')
    plt.ylabel('Frequency')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()

create_bar_chart(top_5_usernames, 'Top 5 Most Common Usernames')
create_bar_chart(top_5_passwords, 'Top 5 Most Common Passwords')
